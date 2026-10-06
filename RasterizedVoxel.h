#ifndef RASTERIZEDVOXEL_H
#define RASTERIZEDVOXEL_H

/**
 * Un voxel traversé par un segment et la distance t
 * depuis le point de départ du segment.
 */
template<typename Point> 
struct RasterizedVoxel
{
  Point voxel;
  double t;
};

#endif
