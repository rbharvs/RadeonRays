/**********************************************************************
Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
********************************************************************/
/**
 \file intersector.h
 \author Dmitry Kozlov
 \version 1.0
 \brief Intersector interface declaration.
 */
 
#pragma once
#include "radeon_rays.h"
#include "calc.h"
#include "buffer.h"
#include "event.h"

#include <functional>
#include <memory>

namespace RadeonRays
{
    class World;

    /** 
    \brief Intersector interface

    Intersector encapsulate the logic of finding batched ray intersection and occlusion queries for a given world.
    Before calling any of QueryXXX methods, the user should call Intersector::Preprocess(world). This methods might
    take significant time to finish since it includes acceleration structure builds, etc. Not all worlds are compatible
    with all intersectors. Before calling Preprocess() the user must make sure the world is compatible using 
    IsCompatible() method.
    */
    class Intersector
    {
    public:
        // Constructor accepts Calc::Device paramter 
        // which is going to be used by an intersector.
        Intersector(Calc::Device* device);
        // Destructor.
        virtual ~Intersector() = default;

        /** 
        \brief Check if the intersector is compatible with a given world.

        World might contain different types of shapes and not all intersectors are compatible with all shapes.
        */
        bool IsCompatible(World const& world) const;

        /** 
        \brief Perform world preprocessing.

        Build all necessary acceleration structures and prepare for queries. Might take significant CPU time.
        */
        void SetWorld(World const& world);

        /** 
        \brief Query intersection for a batch of rays

        The function is asynchronous and returns immediately. The result is available as soon as event is signaled.
        wait_event is being used in order to create CPU-async dependency chains.

        \param queue_idx Device queue index.
        \param rays Ray buffer.
        \param num_rays Number of rays in the buffer.
        \param hits Hit data buffer.
        \param wait_event Event to wait for before execution.
        \param event Completion event.
        */
        void QueryIntersection(std::uint32_t queue_idx, Calc::Buffer const* rays, std::uint32_t num_rays,
            Calc::Buffer* hits, Calc::Event const* wait_event, Calc::Event** event) const;

        /** 
        \brief Query occlusion for a batch of rays

        The function is asynchronous and returns immediately. The result is available as soon as event is signaled.
        wait_event is being used in order to create CPU-async dependency chains.

        \param queue_idx Device queue index.
        \param rays Ray buffer.
        \param num_rays Number of rays in the buffer.
        \param hits Hit data buffer.
        \param wait_event Event to wait for before execution.
        \param event Completion event.
        */
        void QueryOcclusion(std::uint32_t queue_idx, Calc::Buffer const* rays, std::uint32_t num_rays,
            Calc::Buffer* hits, Calc::Event const* wait_event, Calc::Event** event) const;
        
        void QueryOccluded2dSumLinear2(std::uint32_t queue_idx, Calc::Buffer const *origins, Calc::Buffer const *directions, Calc::Buffer const *koefs, Calc::Buffer const *offset_directions,
                                       Calc::Buffer const *offset_koefs, std::uint32_t num_origins, std::uint32_t num_directions,
                                       std::uint32_t directions_stride, Calc::Buffer *hits, Calc::Event const *wait_event, Calc::Event **event) const;
      
        void QueryOccluded2dCellString(std::uint32_t queue_idx, Calc::Buffer const *origins, Calc::Buffer const *directions,
                                       std::uint32_t num_origins, std::uint32_t num_directions, Calc::Buffer const *cell_string_inds,
                                       std::uint32_t num_cell_strings, Calc::Buffer *hits,
                                       Calc::Event const *wait_event, Calc::Event **event) const;

        /** 
        \brief Query intersection for a batch of rays

        The function is asynchronous and returns immediately. The result is available as soon as event is signaled.
        wait_event is being used in order to create CPU-async dependency chains.

        \param queue_idx Device queue index.
        \param rays Ray buffer.
        \param num_rays Buffer, containing the number of rays in rays buffer.
        \param hits Hit data buffer.
        \param wait_event Event to wait for before execution.
        \param event Completion event.
        */
        void QueryIntersection(std::uint32_t queue_idx, Calc::Buffer const *rays, Calc::Buffer const *num_rays, 
            std::uint32_t max_rays, Calc::Buffer *hits, Calc::Event const *wait_event, Calc::Event **event) const;

        /** 
        \brief Query occlusion for a batch of rays

        The function is asynchronous and returns immediately. The result is available as soon as event is signaled.
        wait_event is being used in order to create CPU-async dependency chains.

        \param queue_idx Device queue index.
        \param rays Ray buffer.
        \param num_rays Buffer, containing the number of rays in rays buffer.
        \param hits Hit data buffer.
        \param wait_event Event to wait for before execution.
        \param event Completion event.
        */
        void QueryOcclusion(std::uint32_t queue_idx, Calc::Buffer const* rays, Calc::Buffer const* num_rays,
            std::uint32_t max_rays, Calc::Buffer* hits, Calc::Event const* wait_event, Calc::Event** event) const;

        // Disallow intersector copies
        Intersector(Intersector const&) = delete;
        Intersector& operator = (Intersector const&) = delete;

    private:
        // Preprocess implementation
        virtual void Process(World const& world) = 0;
        // Compatibility check implemetation
        virtual bool IsCompatibleImpl(World const& world) const;
        // Intersection implementation
        virtual void Intersect(std::uint32_t queue_idx, Calc::Buffer const *rays, Calc::Buffer const *num_rays, 
            std::uint32_t max_rays, Calc::Buffer *hits, 
            Calc::Event const *wait_event, Calc::Event **event) const = 0;
        // Occlusion implementation
        virtual void Occluded(std::uint32_t queue_idx, Calc::Buffer const *rays, Calc::Buffer const *num_rays, 
            std::uint32_t max_rays, Calc::Buffer *hits, 
            Calc::Event const *wait_event, Calc::Event **event) const = 0;
        
        virtual void Occluded2dSumLinear2(std::uint32_t queueidx, Calc::Buffer const *origins, Calc::Buffer const *directions, Calc::Buffer const *koefs,
                                          Calc::Buffer const *offset_directions, Calc::Buffer const *offset_koefs,
                                          Calc::Buffer const *num_origins, Calc::Buffer const *num_directions,
                                          Calc::Buffer const *directions_stride, std::uint32_t maxrays, Calc::Buffer *hits,
                                          Calc::Event const *wait_event, Calc::Event **event) const {}
      
//        virtual void QueryOccluded2dCellString(std::uint32_t queue_idx, Calc::Buffer const *origins, Calc::Buffer const *directions,
//                                               std::uint32_t num_origins, std::uint32_t num_directions, Calc::Buffer const *cell_string_inds,
//                                               std::uint32_t num_cell_strings, Calc::Buffer *hits,
//                                               Calc::Event const *wait_event, Calc::Event **event) const {}
      
        virtual void Occluded2dCellString(std::uint32_t queueidx,
                                          Calc::Buffer const *origins,
                                          Calc::Buffer const *directions,
                                          Calc::Buffer const *num_origins,
                                          Calc::Buffer const *num_directions,
                                          Calc::Buffer const *cell_string_inds,
                                          Calc::Buffer const *num_cell_strings,
                                          std::uint32_t max_ray_batches,
                                          Calc::Buffer *hits,
                                          Calc::Event const *wait_event,
                                          Calc::Event **event) const {}

    protected: 
        // Device to use
        Calc::Device* m_device;
        // Buffer holding ray count
        std::unique_ptr<Calc::Buffer, std::function<void(Calc::Buffer*)>> m_counter;
        std::unique_ptr<Calc::Buffer, std::function<void(Calc::Buffer*)>> m_counter2;
        std::unique_ptr<Calc::Buffer, std::function<void(Calc::Buffer*)>> m_counter3;
    };
}

#ifdef RR_EMBED_KERNELS
#if USE_OPENCL
#    include "kernels_cl.h"
#endif

#if USE_VULKAN
#    include "kernels_vk.h"
#endif
#endif // RR_EMBED_KERNELS

