#ifdef STAN_OPENCL
#include <stan/math/prim/mat/fun/Eigen.hpp>
#include <stan/math/prim/mat.hpp>
#include <stan/math/opencl/rev/copy.hpp>
#include <stan/math/opencl/rev/matrix_cl.hpp>
#include <stan/math/opencl/rev/sub_block.hpp>
#include <stan/math/opencl/zeros.hpp>
#include <gtest/gtest.h>
#include <algorithm>

TEST(MathMatrixCL, sub_block_pass_vari) {
  using stan::math::matrix_d;
  using stan::math::matrix_v;
  using stan::math::var;
  using stan::math::matrix_cl;
  using stan::math::vari;
  using stan::math::matrix_vi;
  vari** d1_vals(stan::math::ChainableStack::instance_->memalloc_.alloc_array<vari*>(9));
  vari** d2_vals(stan::math::ChainableStack::instance_->memalloc_.alloc_array<vari*>(16));

  for (int i = 0; i < 9; i++) {
    d1_vals[i] = new vari(i);
  }
  for (int i = 0; i < 16; i++) {
    d2_vals[i] = new vari(15 - i);
  }
  const matrix_vi d1 = Eigen::Map<matrix_vi>(d1_vals, 3, 3);
  const matrix_vi d2 = Eigen::Map<matrix_vi>(d2_vals, 4, 4);
  matrix_cl<var> d11(d1);
  matrix_cl<var> d22(d2);
  d22.sub_block(d11, 0, 0, 0, 0, 2, 2);
  matrix_d d3 = stan::math::from_matrix_cl(d22.val());
  EXPECT_EQ(0, d3(0, 0));
  EXPECT_EQ(3, d3(0, 1));
  EXPECT_EQ(1, d3(1, 0));
  EXPECT_EQ(4, d3(1, 1));
  stan::math::recover_memory();
}
/**
TEST(MathMatrixCL, sub_block_pass_var) {
  using stan::math::var;
  using stan::math::matrix_v;
  using stan::math::matrix_d;
  using stan::math::matrix_cl;
  using stan::math::from_matrix_cl;
  matrix_v d1;
  matrix_v d2;
  matrix_d d3;
  d1 << 1, 2, 3, 4, 5, 6, 7, 8, 9;
  d2 << 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1;

  matrix_cl<var> d11(d1);
  matrix_cl<var> d22(d2);
  d22.sub_block(d11, 0, 0, 0, 0, 2, 2);
  d3 = from_matrix_cl(d22);
  EXPECT_EQ(1, d3(0, 0));
  EXPECT_EQ(2, d3(0, 1));
  EXPECT_EQ(4, d3(1, 0));
  EXPECT_EQ(5, d3(1, 1));
}

TEST(MathMatrixCL, sub_block_exception) {
  using stan::math::matrix_d;
  using stan::math::matrix_v;
  using stan::math::var;
  using stan::math::matrix_cl;
  using stan::math::vari;
  using stan::math::matrix_vi;
  stan::math::matrix_v d1;
  stan::math::matrix_v d2;

  d1.resize(3, 3);
  d2.resize(4, 4);
  d1 << 1, 2, 3, 4, 5, 6, 7, 8, 9;
  d2 << 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1;
  stan::math::matrix_cl<var> d11(d1);
  stan::math::matrix_cl<var> d22(d2);
  EXPECT_THROW(d22.sub_block(d11, 1, 1, 0, 0, 4, 4), std::domain_error);
  EXPECT_THROW(d22.sub_block(d11, 4, 4, 0, 0, 2, 2), std::domain_error);
}
*/

TEST(MathMatrixCL, sub_block_pass_vari2) {
  using stan::math::matrix_d;
  using stan::math::matrix_v;
  using stan::math::var;
  using stan::math::matrix_cl;
  using stan::math::vari;
  using stan::math::matrix_vi;
  vari** d1_vals(stan::math::ChainableStack::instance_->memalloc_.alloc_array<vari*>(9));
  vari** d2_vals(stan::math::ChainableStack::instance_->memalloc_.alloc_array<vari*>(16));

  for (int i = 0; i < 9; i++) {
    d1_vals[i] = new vari(i);
  }
  for (int i = 0; i < 16; i++) {
    d2_vals[i] = new vari(15 - i);
  }
  const matrix_vi d1 = Eigen::Map<matrix_vi>(d1_vals, 3, 3);
  const matrix_vi d2 = Eigen::Map<matrix_vi>(d2_vals, 4, 4);
  matrix_cl<var> d11(d1, stan::math::matrix_cl_view::Lower);
  matrix_cl<var> d22(d2);
  d22.sub_block(d11, 0, 0, 0, 0, 3, 3);
  matrix_d d3 = stan::math::from_matrix_cl(d22.val());
  EXPECT_EQ(0, d3(0, 0));
  EXPECT_EQ(0, d3(0, 1));
  EXPECT_EQ(0, d3(0, 2));
  EXPECT_EQ(1, d3(1, 0));
  EXPECT_EQ(4, d3(1, 1));
  EXPECT_EQ(0, d3(1, 2));
  EXPECT_EQ(2, d3(2, 0));
  EXPECT_EQ(5, d3(2, 1));
  EXPECT_EQ(8, d3(2, 2));
  stan::math::recover_memory();
}



#endif
