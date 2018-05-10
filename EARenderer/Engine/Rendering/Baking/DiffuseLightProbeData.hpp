//
//  DiffuseLightProbeData.hpp
//  EARenderer
//
//  Created by Pavel Muratov on 5/10/18.
//  Copyright © 2018 MPO. All rights reserved.
//

#ifndef DiffuseLightProbeData_hpp
#define DiffuseLightProbeData_hpp

#include "DiffuseLightProbe.hpp"
#include "SurfelClusterProjection.hpp"
#include "GLBufferTexture.hpp"
#include "SphericalHarmonics.hpp"

#include <vector>
#include <memory>

namespace EARenderer {

    class DiffuseLightProbeGenerator;

    class DiffuseLightProbeData {
    private:
        friend DiffuseLightProbeGenerator;

        std::vector<DiffuseLightProbe> mProbes;
        std::vector<SurfelClusterProjection> mSurfelClusterProjections;

        std::shared_ptr<GLFloat3BufferTexture<SphericalHarmonics>> mProjectionClusterSHsBufferTexture;
        std::shared_ptr<GLUIntegerBufferTexture<uint32_t>> mProjectionClusterIndicesBufferTexture;
        std::shared_ptr<GLUIntegerBufferTexture<uint32_t>> mProbeClusterProjectionsMetadataBufferTexture;

    public:
        const auto& probes() const { return mProbes; }

        const auto& surfelClusterProjections() const { return mSurfelClusterProjections; }

        auto projectionClusterSHsBufferTexture() const { return mProjectionClusterSHsBufferTexture; }

        auto projectionClusterIndicesBufferTexture() const { return mProjectionClusterIndicesBufferTexture; };

        auto probeClusterProjectionsMetadataBufferTexture() const { return mProbeClusterProjectionsMetadataBufferTexture; }
    };

}

#endif /* DiffuseLightProbeData_hpp */
