//
// Copyright 2016 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//
#ifndef USDIMAGING_INSTANCE_ADAPTER_H
#define USDIMAGING_INSTANCE_ADAPTER_H

/// \file usdImaging/instanceAdapter.h

#include "pxr/pxr.h"
#include "pxr/usdImaging/usdImaging/primAdapter.h"

#include "pxr/base/tf/hashmap.h"

#include <boost/unordered_map.hpp> 
#include <boost/shared_ptr.hpp> 
#include <boost/enable_shared_from_this.hpp> 

#include <mutex>

PXR_NAMESPACE_OPEN_SCOPE


/// \class UsdImagingInstanceAdapter
///
/// Delegate support for instanced prims.
///
class UsdImagingInstanceAdapter : public UsdImagingPrimAdapter
{
public:
    typedef UsdImagingPrimAdapter BaseAdapter;

    UsdImagingInstanceAdapter();
    virtual ~UsdImagingInstanceAdapter();

    virtual SdfPath Populate(UsdPrim const& prim, UsdImagingIndexProxy* index,
            UsdImagingInstancerContext const* instancerContext = NULL) override;

    virtual bool ShouldCullChildren() const override;

    virtual bool IsInstancerAdapter() const override;

    // ---------------------------------------------------------------------- //
    /// \name Parallel Setup and Resolve
    // ---------------------------------------------------------------------- //
    
    virtual void TrackVariabilityPrep(UsdPrim const& prim,
                                      SdfPath const& cachePath,
                                      UsdImagingInstancerContext const* 
                                          instancerContext = NULL) override;

    /// Thread Safe.
    virtual void TrackVariability(UsdPrim const& prim,
                                  SdfPath const& cachePath,
                                  HdDirtyBits* timeVaryingBits,
                                  UsdImagingInstancerContext const* 
                                      instancerContext = NULL) const override;

    virtual void UpdateForTimePrep(UsdPrim const& prim,
                                   SdfPath const& cachePath, 
                                   UsdTimeCode time,
                                   HdDirtyBits requestedBits,
                                   UsdImagingInstancerContext const* 
                                       instancerContext = NULL) override;

    /// Thread Safe.
    virtual void UpdateForTime(UsdPrim const& prim,
                               SdfPath const& cachePath, 
                               UsdTimeCode time,
                               HdDirtyBits requestedBits,
                               UsdImagingInstancerContext const* 
                                   instancerContext = NULL) const override;

    // ---------------------------------------------------------------------- //
    /// \name Change Processing 
    // ---------------------------------------------------------------------- //

    virtual HdDirtyBits ProcessPropertyChange(UsdPrim const& prim,
                                              SdfPath const& cachePath,
                                              TfToken const& propertyName) override;

    virtual void ProcessPrimResync(SdfPath const& usdPath,
                                   UsdImagingIndexProxy* index) override;

    virtual void ProcessPrimRemoval(SdfPath const& primPath,
                                   UsdImagingIndexProxy* index) override;


    virtual void MarkDirty(UsdPrim const& prim,
                           SdfPath const& cachePath,
                           HdDirtyBits dirty,
                           UsdImagingIndexProxy* index) override;

    // As this adapter hijacks the adapter for the child prim
    // We need to forward these messages on, in case the child
    // adapter needs them
    virtual void MarkRefineLevelDirty(UsdPrim const& prim,
                                      SdfPath const& cachePath,
                                      UsdImagingIndexProxy* index) override;

    virtual void MarkReprDirty(UsdPrim const& prim,
                               SdfPath const& cachePath,
                               UsdImagingIndexProxy* index) override;

    virtual void MarkCullStyleDirty(UsdPrim const& prim,
                                    SdfPath const& cachePath,
                                    UsdImagingIndexProxy* index) override;


    virtual void MarkTransformDirty(UsdPrim const& prim,
                                    SdfPath const& cachePath,
                                    UsdImagingIndexProxy* index) override;

    virtual void MarkVisibilityDirty(UsdPrim const& prim,
                                     SdfPath const& cachePath,
                                     UsdImagingIndexProxy* index) override;



    // ---------------------------------------------------------------------- //
    /// \name Instancing
    // ---------------------------------------------------------------------- //

    virtual SdfPath GetPathForInstanceIndex(SdfPath const &path,
                                            int instanceIndex,
                                            int *instanceCount,
                                            int *absoluteInstanceIndex,
                                            SdfPath * rprimPath=NULL,
                                            SdfPathVector *instanceContext=NULL) override;

    virtual SdfPath GetInstancer(SdfPath const &cachePath) override;

    virtual size_t
    SampleInstancerTransform(UsdPrim const& instancerPrim,
                             SdfPath const& instancerPath,
                             UsdTimeCode time,
                             const std::vector<float>& configuredSampleTimes,
                             size_t maxSampleCount,
                             float *times,
                             GfMatrix4d *samples) override;

    virtual size_t
    SamplePrimvar(UsdPrim const& usdPrim,
                  SdfPath const& cachePath,
                  TfToken const& key,
                  UsdTimeCode time,
                  const std::vector<float>& configuredSampleTimes,
                  size_t maxNumSamples, float *times,
                  VtValue *samples) override;

    // ---------------------------------------------------------------------- //
    /// \name Nested instancing support
    // ---------------------------------------------------------------------- //

    virtual VtIntArray GetInstanceIndices(SdfPath const &instancerPath,
                                          SdfPath const &protoRprimPath) override;

    virtual GfMatrix4d GetRelativeInstancerTransform(
        SdfPath const &parentInstancerPath,
        SdfPath const &instancerPath,
        UsdTimeCode time) const override;

    // ---------------------------------------------------------------------- //
    /// \name Selection
    // ---------------------------------------------------------------------- //
    virtual bool PopulateSelection( 
                                HdSelection::HighlightMode const& highlightMode,
                                SdfPath const &path,
                                VtIntArray const &instanceIndices,
                                HdSelectionSharedPtr const &result) override;

    // ---------------------------------------------------------------------- //
    /// \name Utilities
    // ---------------------------------------------------------------------- //

    /// Returns the depending rprim paths which don't exist in descendants.
    /// Used for change tracking over subtree boundary (e.g. instancing)
    virtual SdfPathVector GetDependPaths(SdfPath const &path) const override;

protected:
    virtual void _RemovePrim(SdfPath const& cachePath,
                             UsdImagingIndexProxy* index) override final;

private:

    SdfPath _Populate(UsdPrim const& prim,
                      UsdImagingIndexProxy* index,
                      UsdImagingInstancerContext const* instancerContext,
                      SdfPath const& parentProxyPath);

    struct _ProtoRprim;
    struct _ProtoGroup;

    bool _IsChildPrim(UsdPrim const& prim,
                      SdfPath const& cachePath) const;

    UsdImagingPrimAdapterSharedPtr _GetSharedFromThis();

    // Returns true if the given prim serves as an instancer.
    bool _PrimIsInstancer(UsdPrim const& prim) const;

    // Inserts an rprim for the given \p usdPrim into the \p index.
    // The rprim will be created under a child path combining the 
    // \p instancerPath and \p protoName; that path will be returned.
    SdfPath 
    _InsertProtoRprim(UsdPrimRange::iterator *iter,
                      const TfToken& protoName,
                      SdfPath materialId,
                      TfToken drawMode,
                      SdfPath instancerPath,
                      UsdImagingPrimAdapterSharedPtr const& primAdapter,
                      UsdImagingPrimAdapterSharedPtr const& instancerAdapter,
                      UsdImagingIndexProxy* index,
                      bool *isLeafInstancer);

    // For a usd path, collects the instancers to resync.
    void _ResyncPath(SdfPath const& usdPath,
                     UsdImagingIndexProxy* index,
                     bool reload);
    // Removes and optionally reloads all instancer data, both locally and
    // from the render index.
    void _ResyncInstancer(SdfPath const& instancerPath,
                          UsdImagingIndexProxy* index, bool reload);

    // Updates per-frame data in the instancer map. This is primarily used
    // during update to send new instance indices out to Hydra.
    struct _UpdateInstanceMapFn;
    void _UpdateInstanceMap(UsdPrim const& instancerPrim, 
            UsdTimeCode time) const;

    // Update the dirty bits per-instancer. This is only executed once per
    // instancer, this method uses the instancer mutex to avoid redundant work.
    //
    // Returns the instancer's dirty bits.
    int _UpdateDirtyBits(UsdPrim const& instancerPrim) const;

    // Gets the associated _ProtoRprim and instancer context for the given 
    // instancer and cache path.
    _ProtoRprim const& _GetProtoRprim(SdfPath const& instancerPath, 
                                      SdfPath const& cachePath,
                                      UsdImagingInstancerContext* ctx) const;

    // Computes the transforms for all instances corresponding to the given
    // instancer.
    struct _ComputeInstanceTransformFn;
    bool _ComputeInstanceTransforms(UsdPrim const& instancer,
                                    VtMatrix4dArray* transforms,
                                    UsdTimeCode time) const;

    // Returns true if any of the instances corresponding to the given
    // instancer has a varying transform.
    struct _IsInstanceTransformVaryingFn;
    bool _IsInstanceTransformVarying(UsdPrim const& instancer) const;

    struct _GetPathForInstanceIndexFn;
    struct _PopulateInstanceSelectionFn;

    // Helper functions for dealing with "actual" instances to be drawn.
    //
    // Suppose we have:
    //    /Root
    //        Instance_A (master: /__Master_1)
    //        Instance_B (master: /__Master_1)
    //    /__Master_1
    //        AnotherInstance_A (master: /__Master_2)
    //    /__Master_2
    //
    // /__Master_2 has only one associated instance in the Usd scenegraph: 
    // /__Master_1/AnotherInstance_A. However, imaging actually needs to draw 
    // two instances of /__Master_2, because AnotherInstance_A is a nested 
    // instance beneath /__Master_1, and there are two instances of /__Master_1.
    //
    // Each instance to be drawn is addressed by the chain of instances
    // that caused it to be drawn. In the above example, the two instances 
    // of /__Master_2 to be drawn are:
    //
    //  [ /Root/Instance_A, /__Master_1/AnotherInstance_A ],
    //  [ /Root/Instance_B, /__Master_1/AnotherInstance_A ]
    //
    // This "instance context" describes the chain of opinions that
    // ultimately affect the final drawn instance. For example, the 
    // transform of each instance to draw is the combined transforms
    // of the prims in each context.
    template <typename Functor>
    void _RunForAllInstancesToDraw(UsdPrim const& instancer, Functor* fn) const;
    template <typename Functor>
    bool _RunForAllInstancesToDrawImpl(UsdPrim const& instancer, 
                                       std::vector<UsdPrim>* instanceContext,
                                       size_t* instanceIdx,
                                       Functor* fn) const;

    typedef TfHashMap<SdfPath, size_t, SdfPath::Hash> _InstancerDrawCounts;
    size_t _CountAllInstancesToDraw(UsdPrim const& instancer) const;
    size_t _CountAllInstancesToDrawImpl(UsdPrim const& instancer,
                                        _InstancerDrawCounts* drawCounts) const;

    // A proto group represents a complete set of rprims for a given prototype
    // declared on the instancer.
    struct _ProtoGroup {
        // The time at which the instance data should be fetched.
        UsdTimeCode time;
        // A vector of prototype indices that also index into the primvar data.
        // All elements in this array can be dispatched as a single hardware
        // draw call (though this is a detail of the renderer implementation).
        VtIntArray indices;
    };
    typedef boost::shared_ptr<_ProtoGroup> _ProtoGroupPtr;

    // A proto rprim represents a single rprim under a prototype root declared
    // on the instancer. For example, a character may be targeted by the
    // prototypes relationship, which will have many meshes, each mesh is
    // represented as a proto rprim.
    struct _ProtoRprim {
        _ProtoRprim() : variabilityBits(0), visible(true) {}
        // Each rprim will become a prototype "child" under the instancer. This
        // path is the path to the gprim on the Usd Stage (the path to a single
        // mesh, for example).
        SdfPath path;           
        // The prim adapter for the actual prototype gprim.
        UsdImagingPrimAdapterSharedPtr adapter;
        // The prototype group that this rprim belongs to.
        // Over time, as instances are animated, multiple copies of the
        // prototype may be required to, for example, draw two different frames
        // of animation. This ID maps the rprim its associated instance data
        // over time.
        _ProtoGroupPtr protoGroup;
        // Tracks the variability of the underlying adapter to avoid
        // redundantly reading data. This value is stored as
        // HdDirtyBits flags.
        HdDirtyBits variabilityBits;
        // When variabilityBits does not include HdChangeTracker::DirtyVisibility
        // the visible field is the unvarying value for visibility.
        bool visible;
    };

    // Indexed by cachePath (each rprim has one entry)
    typedef TfHashMap<SdfPath, _ProtoRprim, SdfPath::Hash> _PrimMap;

    // All data associated with a given instancer prim. PrimMap could
    // technically be split out to avoid two lookups, however it seems cleaner
    // to keep everything bundled up under the instancer path.
    struct _InstancerData {
        _InstancerData() : numInstancesToDraw(0) { }

        // The master prim path associated with this instancer.
        SdfPath masterPath;

        // The material path associated with this instancer.
        SdfPath materialId;

        // The drawmode associated with this instancer.
        TfToken drawMode;

        // Paths to Usd instance prims. Note that this is not necessarily
        // equivalent to all the instances that will be drawn. See below.
        std::vector<SdfPath> instancePaths;

        // Number of actual instances of this instancer that will be 
        // drawn. See comment on _RunForAllInstancesToDraw.
        size_t numInstancesToDraw;

        // Cached visibility. This vector contains an entry for each instance
        // that will be drawn (i.e., visibility.size() == numInstancesToDraw).
        enum Visibility {
            Invisible, //< Invisible over all time
            Visible,   //< Visible over all time
            Varying,   //< Visibility varies over time
            Unknown    //< Visibility has not yet been checked
        };
        std::vector<Visibility> visibility;

        // Map of all rprims for this instancer prim.
        _PrimMap primMap;

        // This is a set of reference paths, where this instancer needs
        // to deferer to another instancer.  While refered to here as a child
        // instancer, the actual relationship is more like a directed graph.
        SdfPathSet childInstancers;

        // Proto group containing the instance indexes for each prototype
        // rprim.
        _ProtoGroupPtr protoGroup;

        // Instancer dirty bits.
        HdDirtyBits dirtyBits;

        std::mutex mutex;
    };

    // Map from instancer path to instancer data.
    // Note: this map is modified in multithreaded code paths and must be
    // locked.
    typedef boost::unordered_map<SdfPath, _InstancerData, SdfPath::Hash> 
        _InstancerDataMap;
    mutable _InstancerDataMap _instancerData;

    // Map from instance to instancer.
    // XXX: consider to move this forwarding map into HdRenderIndex.
    typedef TfHashMap<SdfPath, SdfPath, SdfPath::Hash>
        _InstanceToInstancerMap;
    _InstanceToInstancerMap _instanceToInstancerMap;

    // Hd and UsdImaging think of instancing in terms of an 'instancer' that
    // specifies a list of 'prototype' prims that are shared per instance.
    //
    // For Usd scenegraph instancing, a master prim and its descendents
    // roughly correspond to the instancer and prototype prims. However,
    // Hd requires a different instancer and rprims for different combinations
    // of inherited attributes (material binding, draw mode, etc).
    // This means we cannot use the Usd master prim as the instancer, because
    // we can't represent this in the case where multiple Usd instances share
    // the same master but have different bindings.
    //
    // Instead, we use the first instance of a master with a given set of
    // inherited attributes as our instancer. For example, if /A and /B are
    // both instances of /__Master_1 but /A and /B have different material
    // bindings authored on them, both /A and /B will be instancers,
    // with their own set of rprims and instance indices.
    //
    // The below is a multimap from master path to instancer path. The data
    // for the instancer is located in the _InstancerDataMap above.
    typedef TfHashMultiMap<SdfPath, SdfPath, SdfPath::Hash>
        _MasterToInstancerMap;
    _MasterToInstancerMap _masterToInstancerMap;
};


PXR_NAMESPACE_CLOSE_SCOPE

#endif // USDIMAGING_INSTANCE_ADAPTER_H
