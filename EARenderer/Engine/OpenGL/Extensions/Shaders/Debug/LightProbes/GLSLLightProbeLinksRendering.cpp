//
//  GLSLLightProbeLinksRendering.cpp
//  EARenderer
//
//  Created by Pavel Muratov on 4/3/18.
//  Copyright © 2018 MPO. All rights reserved.
//

#include "GLSLLightProbeLinksRendering.hpp"

namespace EARenderer {

#pragma mark - Lifecycle

    GLSLLightProbeLinksRendering::GLSLLightProbeLinksRendering()
            :
            GLProgram("LightProbeLinksRendering.vert", "LightProbeLinksRendering.frag", "LightProbeLinksRendering.geom") {
    }

#pragma mark - Setters

    void GLSLLightProbeLinksRendering::setCamera(const Camera &camera) {
        glUniformMatrix4fv(uniformByNameCRC32(ctcrc32("uCameraSpaceMat")).location(), 1, GL_FALSE, glm::value_ptr(camera.viewProjectionMatrix()));
    }

    void GLSLLightProbeLinksRendering::setWorldBoundingBox(const AxisAlignedBox3D &box) {
        glUniformMatrix4fv(uniformByNameCRC32(ctcrc32("uWorldBoudningBoxTransform")).location(), 1, GL_FALSE, glm::value_ptr(box.localSpaceMatrix()));
    }

    void GLSLLightProbeLinksRendering::setProjectionClusterIndices(const GLIntegerBufferTexture<GLTexture::Integer::R32UI, uint32_t> &indices) {
        setBufferTexture(ctcrc32("uProjectionClusterIndices"), indices);
    }

    void GLSLLightProbeLinksRendering::setProbeProjectionsMetadata(const GLIntegerBufferTexture<GLTexture::Integer::R32UI, uint32_t> &metadata) {
        setBufferTexture(ctcrc32("uProbeProjectionsMetadata"), metadata);
    }

    void GLSLLightProbeLinksRendering::setSurfelClusterCenters(const GLFloatBufferTexture<GLTexture::Float::RGB32F, glm::vec3> &centers) {
        setBufferTexture(ctcrc32("uClusterCenters"), centers);
    }

    void GLSLLightProbeLinksRendering::setProbesGridResolution(const glm::ivec3 &resolution) {
        glUniform3iv(uniformByNameCRC32(ctcrc32("uProbesGridResolution")).location(), 1, glm::value_ptr(resolution));
    }

}
