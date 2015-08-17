/*
   Copyright 2012 Brain Research Institute, Melbourne, Australia

   Written by David Raffelt, 24/02/2012

   This file is part of MRtrix.

   MRtrix is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   MRtrix is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MRtrix.  If not, see <http://www.gnu.org/licenses/>.

 */

#ifndef __image_registration_symmetric_metric_evaluate_h__
#define __image_registration_symmetric_metric_evaluate_h__

#include "image/registration_symmetric/metric/thread_kernel.h"
#include "image/threaded_loop.h"
#include "math/matrix.h"
#include "image/registration/transform/reorient.h"
#include "image/buffer_scratch.h"
#include "image/voxel.h"

namespace MR
{
  namespace Image
  {
    namespace RegistrationSymmetric
    {
      namespace Metric
      {
        template <class MetricType, class ParamType>
          class Evaluate {
            public:

              typedef typename ParamType::TransformParamType TransformParamType;
              typedef double value_type;

              Evaluate (const MetricType& metric, ParamType& parameters) :
                metric (metric),
                params (parameters),
                iteration (1) { }


              double operator() (const Math::Vector<double>& x, Math::Vector<double>& gradient) {

                double overall_cost_function = 0.0;
                gradient.zero();
                params.transformation.set_parameter_vector(x);

                std::unique_ptr<Image::BufferScratch<float> > reoriented_moving;
                std::unique_ptr<Image::BufferScratch<float>::voxel_type > reoriented_moving_vox;

                if (directions.is_set()) {
                  throw Exception("TODO: make directions symmetric");
                  reoriented_moving.reset (new Image::BufferScratch<float> (params.moving_image));
                  reoriented_moving_vox.reset (new Image::BufferScratch<float>::voxel_type (*reoriented_moving));
                  Image::Registration::Transform::reorient (params.moving_image, *reoriented_moving_vox, params.transformation.get_matrix(), directions);
                  params.set_moving_interpolator (*reoriented_moving_vox);
                  metric.set_moving_image (*reoriented_moving_vox);

                }

                { 
                  ThreadKernel<MetricType, ParamType> kernel (metric, params, overall_cost_function, gradient);
                  Image::ThreadedLoop (params.midway_image, 0, 3).run (kernel);
                }
                // std::cerr.precision(10);
                DEBUG("Metric evaluate iteration: " + str(iteration++) + ", cost: " +str(overall_cost_function));
                return overall_cost_function;
              }

              void set_directions (Math::Matrix<float>& dir) {
                directions = dir;
              }

              size_t size() {
                return params.transformation.size();
              }

              double init (Math::Vector<TransformParamType>& x) {
                params.transformation.get_parameter_vector(x);
                return 1.0;
              }

            protected:
                MetricType metric;
                ParamType params;
                Math::Matrix<float> directions;
                std::vector<size_t> extent;
                size_t iteration;

        };
      }
    }
  }
}

#endif
