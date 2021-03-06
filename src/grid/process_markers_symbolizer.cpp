/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/

#if defined(GRID_RENDERER)

/*

porting notes -->

 - rasterizer -> grid_rasterizer
 - current_buffer_ -> pixmap_
 - agg::rendering_buffer -> grid_renderering_buffer
 - no gamma
 - agg::scanline_bin sl
 - grid_rendering_buffer
 - agg::renderer_scanline_bin_solid
 - TODO - clamp sizes to > 4 pixels of interactivity
 - svg_renderer.render_id
 - only encode feature if placements are found:
    if (placed)
    {
        pixmap_.add_feature(feature);
    }

*/

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/grid/grid_rasterizer.hpp>
#include <mapnik/grid/grid_renderer.hpp>
#include <mapnik/grid/grid_renderer_base.hpp>
#include <mapnik/grid/grid.hpp>
#include <mapnik/grid/grid_marker_helpers.hpp>

#include <mapnik/debug.hpp>
#include <mapnik/geom_util.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/marker_helpers.hpp>
#include <mapnik/svg/svg_renderer_agg.hpp>
#include <mapnik/svg/svg_storage.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/renderer_common/process_markers_symbolizer.hpp>

// agg
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_rasterizer_scanline_aa.h"

// boost
#include <boost/optional.hpp>

// stl
#include <algorithm>
#include <tuple>

namespace mapnik {

template <typename T>
void grid_renderer<T>::process(markers_symbolizer const& sym,
                               mapnik::feature_impl & feature,
                               proj_transform const& prj_trans)
{
    typedef grid_rendering_buffer buf_type;
    typedef typename grid_renderer_base_type::pixfmt_type pixfmt_type;
    typedef agg::renderer_scanline_bin_solid<grid_renderer_base_type> renderer_type;
    typedef label_collision_detector4 detector_type;

    using namespace mapnik::svg;
    typedef agg::pod_bvector<path_attributes> svg_attribute_type;
    typedef svg_renderer_agg<svg_path_adapter,
                             svg_attribute_type,
                             renderer_type,
                             pixfmt_type > svg_renderer_type;

    buf_type render_buf(pixmap_.raw_data(), common_.width_, common_.height_, common_.width_);
    ras_ptr->reset();
    box2d<double> clip_box = common_.query_extent_;

    auto renderer_context = std::tie(render_buf,*ras_ptr,pixmap_);
    typedef decltype(renderer_context) context_type;

    typedef vector_markers_rasterizer_dispatch_grid<svg_renderer_type,
                                                    detector_type,
                                                    context_type> vector_dispatch_type;

    typedef raster_markers_rasterizer_dispatch_grid<grid_renderer_base_type,
                                                    renderer_type,
                                                    detector_type,
                                                    context_type> raster_dispatch_type;

    render_markers_symbolizer<vector_dispatch_type, raster_dispatch_type>(
        sym, feature, prj_trans, common_, clip_box,renderer_context);
}

template void grid_renderer<grid>::process(markers_symbolizer const&,
                                           mapnik::feature_impl &,
                                           proj_transform const&);
}

#endif
