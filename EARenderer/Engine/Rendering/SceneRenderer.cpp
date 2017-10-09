//
//  Renderer.cpp
//  EARenderer
//
//  Created by Pavlo Muratov on 29.03.17.
//  Copyright © 2017 MPO. All rights reserved.
//

#include "SceneRenderer.hpp"
#include "GLShader.hpp"
#include "Vertex1P4.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace EARenderer {
    
#pragma mark - Lifecycle
    
    SceneRenderer::SceneRenderer(Scene* scene)
    :
    mScene(scene),
    mCascadedShadowMaps(Size2D(2048, 2048), mNumberOfCascades),
    mShadowCubeMap(Size2D(2048, 2048)),
    mDepthFramebuffer(Size2D(2048, 2048))
    { }
    
#pragma mark - Setters
    
    void SceneRenderer::setDefaultRenderComponentsProvider(DefaultRenderComponentsProviding *provider) {
        mDefaultRenderComponentsProvider = provider;
    }
    
    void SceneRenderer::setMeshHiglightEnabled(bool enabled, ID meshID) {
        if (enabled) {
            mMeshesToHighlight.insert(meshID);
        } else {
            mMeshesToHighlight.erase(meshID);
        }
    }
    
    void SceneRenderer::disableMeshesHighlight() {
        mMeshesToHighlight.clear();
    }
    
#pragma mark - Math
    
    bool SceneRenderer::raySelectsMesh(const Ray3D& ray, ID& meshID) {
        float minimumDistance = std::numeric_limits<float>::max();
        ID closestMeshID = IDNotFound;
        
        for (ID id : mScene->meshes()) {
            Mesh& mesh = mScene->meshes()[id];
            Transformation& transformation = mScene->transforms()[mesh.transformID()];
            glm::mat4 modelMatrix = transformation.modelMatrix();
            Ray3D meshLocalSpaceRay = ray.transformedBy(glm::inverse(modelMatrix));
            
            float distance = 0;
            if (meshLocalSpaceRay.intersectsAAB(mesh.boundingBox(), distance)) {
                // Intersection distance is in the mesh's local space
                // Scale local space ray's direction vector (which is a unit vector) accordingly
                glm::vec3 localScaledDirection = meshLocalSpaceRay.direction * distance;
                // Transform back to world space to obtain real origin -> intersection distance
                glm::vec3 worldScaledDirection = modelMatrix * glm::vec4(localScaledDirection, 1.0);
                float worldDistance = glm::length(worldScaledDirection);
                
                if (worldDistance < minimumDistance) {
                    minimumDistance = worldDistance;
                    closestMeshID = id;
                }
            }
        }
        
        meshID = closestMeshID;
        return closestMeshID != IDNotFound;
    }
    
#pragma mark - Rendering
    
    void SceneRenderer::renderSkybox() {
        mSkyboxShader.bind();
        mSkyboxShader.setViewMatrix(mScene->camera()->viewMatrix());
        mSkyboxShader.setProjectionMatrix(mScene->camera()->projectionMatrix());
        mSkyboxShader.setCubemap(mScene->skybox()->cubemap());
        mScene->skybox()->draw();
    }
    
#pragma mark Blinn-Phong
    
    void SceneRenderer::renderShadowMapsForDirectionalLights(const FrustumCascades& cascades) {
        // Fill shadow map
        mDirectionalDepthShader.bind();
        
        for (uint8_t cascade = 0; cascade < cascades.splits.size(); cascade++) {
            mDepthFramebuffer.attachTextureLayer(mCascadedShadowMaps, cascade);
            glClear(GL_DEPTH_BUFFER_BIT);
            
            for (ID subMeshID : mScene->subMeshes()) {
                SubMesh& subMesh = mScene->subMeshes()[subMeshID];
                ID meshID = subMesh.meshID();
                ID transformID = mScene->meshes()[meshID].transformID();
                Transformation& transform = mScene->transforms()[transformID];
                
                mDirectionalDepthShader.setModelMatrix(transform.modelMatrix());
                mDirectionalDepthShader.setViewProjectionMatrix(cascades.lightViewProjections[cascade]);

                subMesh.draw();
            }
        }
    }
    
    void SceneRenderer::renderShadowMapsForPointLights() {
        mOmnidirectionalDepthShader.bind();
        mDepthFramebuffer.attachTexture(mShadowCubeMap);
        
        glClear(GL_DEPTH_BUFFER_BIT);
        
        for (ID subMeshID : mScene->subMeshes()) {
            SubMesh& subMesh = mScene->subMeshes()[subMeshID];
            ID meshID = subMesh.meshID();
            ID transformID = mScene->meshes()[meshID].transformID();
            Transformation& transform = mScene->transforms()[transformID];
            ID lightID = *(mScene->pointLights().begin());
            PointLight& light = mScene->pointLights()[lightID];
            
            mOmnidirectionalDepthShader.setModelMatrix(transform.modelMatrix());
            mOmnidirectionalDepthShader.setLight(light);
            
            subMesh.draw();
        }
    }
    
    void SceneRenderer::renderDirectionalLighting(const FrustumCascades& cascades) {
        ID lightID = *(mScene->directionalLights().begin());
        DirectionalLight& light = mScene->directionalLights()[lightID];
        
        mDirectionalBlinnPhongShader.bind();
        
        for (ID subMeshID : mScene->subMeshes()) {
            mDirectionalBlinnPhongShader.ensureSamplerValidity([&]() {
                mDirectionalBlinnPhongShader.setCamera(*(mScene->camera()));
                mDirectionalBlinnPhongShader.setDirectionalLight(light);
                mDirectionalBlinnPhongShader.setShadowMaps(mCascadedShadowMaps);
                mDirectionalBlinnPhongShader.setShadowCascades(cascades);
                
                SubMesh& subMesh = mScene->subMeshes()[subMeshID];
                ID meshID = subMesh.meshID();
                Mesh& mesh = mScene->meshes()[meshID];
                ID transformID = mesh.transformID();
                Transformation& transform = mScene->transforms()[transformID];
                
                mDirectionalBlinnPhongShader.setModelMatrix(transform.modelMatrix());
                
                ID materialID = *(mScene->classicMaterials().begin());
                ClassicMaterial& material = mScene->classicMaterials()[materialID];
                
                mDirectionalBlinnPhongShader.setMaterial(material);
                mDirectionalBlinnPhongShader.setHighlightColor(mesh.isHighlighted() ? Color::gray() : Color::black());
                
                subMesh.draw();
            });
        }
    }
    
    void SceneRenderer::renderPointLighting() {
        mOmnidirectionalBlinnPhongShader.bind();
        
        for (ID subMeshID : mScene->subMeshes()) {
            mOmnidirectionalBlinnPhongShader.ensureSamplerValidity([&]() {
                SubMesh& subMesh = mScene->subMeshes()[subMeshID];
                ID meshID = subMesh.meshID();
                Mesh& mesh = mScene->meshes()[meshID];
                ID transformID = mesh.transformID();
                Transformation& transform = mScene->transforms()[transformID];
                
                ID lightID = *(mScene->directionalLights().begin());
                PointLight& light = mScene->pointLights()[lightID];
                
                mOmnidirectionalBlinnPhongShader.setModelMatrix(transform.modelMatrix());
                mOmnidirectionalBlinnPhongShader.setCamera(*(mScene->camera()));
                mOmnidirectionalBlinnPhongShader.setPointLight(light);
                mOmnidirectionalBlinnPhongShader.setShadowMap(mShadowCubeMap);
                
                ID materialID = *(mScene->classicMaterials().begin());
                ClassicMaterial& material = mScene->classicMaterials()[materialID];
                
                mOmnidirectionalBlinnPhongShader.setMaterial(material);
                
                subMesh.draw();
            });
        }
    }
    
    void SceneRenderer::renderClassicMeshes() {
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glClearColor(0.0, 0.0, 0.0, 1.0);
        
        mDepthFramebuffer.bind();
        mDepthFramebuffer.viewport().apply();
        
        ID lightID = *(mScene->directionalLights().begin());
        DirectionalLight& light = mScene->directionalLights()[lightID];
        FrustumCascades cascades = light.cascadesForCamera(*mScene->camera(), mNumberOfCascades);
        
        renderShadowMapsForDirectionalLights(cascades);
        renderShadowMapsForPointLights();
        
        if (mDefaultRenderComponentsProvider) {
            mDefaultRenderComponentsProvider->bindSystemFramebuffer();
            mDefaultRenderComponentsProvider->defaultViewport().apply();
        }
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //
        renderDirectionalLighting(cascades);
        renderPointLighting();
        //        renderSkybox();
    }
    
#pragma mark Cook-Torrance
    
    void SceneRenderer::renderPBRMeshes() {
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glClearColor(0.0, 0.0, 0.0, 1.0);
        
        if (mDefaultRenderComponentsProvider) {
            mDefaultRenderComponentsProvider->bindSystemFramebuffer();
            mDefaultRenderComponentsProvider->defaultViewport().apply();
        }
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        ID lightID = *(mScene->pointLights().begin());
        PointLight& light = mScene->pointLights()[lightID];
        
        mCookTorranceShader.bind();
        
        for (ID subMeshID : mScene->subMeshes()) {
            mCookTorranceShader.ensureSamplerValidity([&]() {
                mCookTorranceShader.setCamera(*(mScene->camera()));
                mCookTorranceShader.setLight(light);
                
                SubMesh& subMesh = mScene->subMeshes()[subMeshID];
                ID meshID = subMesh.meshID();
                Mesh& mesh = mScene->meshes()[meshID];
                ID transformID = mesh.transformID();
                Transformation& transform = mScene->transforms()[transformID];
                
                mCookTorranceShader.setModelMatrix(transform.modelMatrix());
                
                ID materialID = *(mScene->PBRMaterials().begin());
                PBRMaterial& material = mScene->PBRMaterials()[materialID];
                
                mCookTorranceShader.setMaterial(material);
                
                subMesh.draw();
            });
        }
    }
    
#pragma mark - Public access point
    
    void SceneRenderer::render() {
        renderPBRMeshes();
    }
    
}