#ifdef STAN_OPENCL
#include <stan/math.hpp>
#include <stan/math/opencl/opencl.hpp>
#include <gtest/gtest.h>
#include <test/unit/math/expect_near_rel.hpp>
#include <vector>

using Eigen::Array;
using Eigen::Dynamic;
using Eigen::Matrix;
using stan::math::matrix_cl;
using stan::math::var;
using stan::test::expect_near_rel;
using std::vector;

TEST(ProbDistributionsPoissonLogGLM, error_checking) {
  double eps = 1e-9;
  int N = 3;
  int M = 2;

  vector<int> y{0, 1, 5};
  vector<int> y_size{0, 1, 5, 0};
  vector<int> y_value{1, 4, -23};
  Matrix<double, Dynamic, Dynamic> x(N, M);
  x << -12, 46, -42, 24, 25, 27;
  Matrix<double, Dynamic, Dynamic> x_size1(N - 1, M);
  x_size1 << -12, 46, -42, 24;
  Matrix<double, Dynamic, Dynamic> x_size2(N, M - 1);
  x_size2 << -12, 46, -42;
  Matrix<double, Dynamic, Dynamic> x_value(N, M);
  x_value << -12, 46, -42, 24, 25, -INFINITY;
  Matrix<double, Dynamic, 1> beta(M, 1);
  beta << 0.3, 2;
  Matrix<double, Dynamic, 1> beta_size(M + 1, 1);
  beta_size << 0.3, 2, 0.4;
  Matrix<double, Dynamic, 1> beta_value(M, 1);
  beta_value << 0.3, INFINITY;
  Matrix<double, Dynamic, 1> alpha(N, 1);
  alpha << 0.3, -0.8, 1.8;
  Matrix<double, Dynamic, 1> alpha_size(N - 1, 1);
  alpha_size << 0.3, -0.8;
  Matrix<double, Dynamic, 1> alpha_value(N, 1);
  alpha_value << 0.3, -0.8, NAN;

  matrix_cl<double> x_cl(x);
  matrix_cl<double> x_size1_cl(x_size1);
  matrix_cl<double> x_size2_cl(x_size2);
  matrix_cl<double> x_value_cl(x_value);
  matrix_cl<int> y_cl(y, N, 1);
  matrix_cl<int> y_size_cl(y_size, N + 1, 1);
  matrix_cl<int> y_value_cl(y_value, N, 1);

  EXPECT_NO_THROW(stan::math::poisson_log_glm_lpmf(y_cl, x_cl, alpha, beta));

  EXPECT_THROW(stan::math::poisson_log_glm_lpmf(y_size_cl, x_cl, alpha, beta),
               std::invalid_argument);
  EXPECT_THROW(stan::math::poisson_log_glm_lpmf(y_cl, x_size1_cl, alpha, beta),
               std::invalid_argument);
  EXPECT_THROW(stan::math::poisson_log_glm_lpmf(y_cl, x_size2_cl, alpha, beta),
               std::invalid_argument);
  EXPECT_THROW(stan::math::poisson_log_glm_lpmf(y_cl, x_cl, alpha_size, beta),
               std::invalid_argument);
  EXPECT_THROW(stan::math::poisson_log_glm_lpmf(y_cl, x_cl, alpha, beta_size),
               std::invalid_argument);
  EXPECT_THROW(stan::math::poisson_log_glm_lpmf(y_cl, x_cl, alpha, beta_size),
               std::invalid_argument);

  EXPECT_THROW(stan::math::poisson_log_glm_lpmf(y_value_cl, x_cl, alpha, beta),
               std::domain_error);
  EXPECT_THROW(stan::math::poisson_log_glm_lpmf(y_cl, x_value_cl, alpha, beta),
               std::domain_error);
  EXPECT_THROW(stan::math::poisson_log_glm_lpmf(y_cl, x_cl, alpha_value, beta),
               std::domain_error);
  EXPECT_THROW(stan::math::poisson_log_glm_lpmf(y_cl, x_cl, alpha, beta_value),
               std::domain_error);
}

TEST(ProbDistributionsPoissonLogGLM, gpu_matches_cpu_small_simple) {
  double eps = 1e-9;
  int N = 3;
  int M = 2;

  vector<int> y{0, 1, 5};
  Matrix<double, Dynamic, Dynamic> x(N, M);
  x << -12, 46, -42, 24, 25, 27;
  Matrix<double, Dynamic, 1> beta(M, 1);
  beta << 0.3, 2;
  double alpha = 0.3;

  matrix_cl<double> x_cl(x);
  matrix_cl<int> y_cl(y, N, 1);

  expect_near_rel("poisson_log_glm_lpmf (OpenCL)",
                  stan::math::poisson_log_glm_lpmf(y_cl, x_cl, alpha, beta),
                  stan::math::poisson_log_glm_lpmf(y, x, alpha, beta));
  expect_near_rel(
      "poisson_log_glm_lpmf (OpenCL)",
      stan::math::poisson_log_glm_lpmf<true>(y_cl, x_cl, alpha, beta),
      stan::math::poisson_log_glm_lpmf<true>(y, x, alpha, beta));

  Matrix<var, Dynamic, 1> beta_var1 = beta;
  Matrix<var, Dynamic, 1> beta_var2 = beta;
  var alpha_var1 = alpha;
  var alpha_var2 = alpha;

  var res1
      = stan::math::poisson_log_glm_lpmf(y_cl, x_cl, alpha_var1, beta_var1);
  var res2 = stan::math::poisson_log_glm_lpmf(y, x, alpha_var2, beta_var2);

  (res1 + res2).grad();

  expect_near_rel("poisson_log_glm_lpmf (OpenCL)", res1.val(), res2.val());

  expect_near_rel("poisson_log_glm_lpmf (OpenCL)", alpha_var1.adj(),
                  alpha_var2.adj());
  expect_near_rel("poisson_log_glm_lpmf (OpenCL)", beta_var1.adj().eval(),
                  beta_var2.adj().eval());
}

TEST(ProbDistributionsPoissonLogGLM, gpu_broadcast_y) {
  double eps = 1e-9;
  int N = 3;
  int M = 2;

  int y = 4;
  vector<int> y_vec{y, y, y};
  Matrix<double, Dynamic, Dynamic> x(N, M);
  x << -12, 46, -42, 24, 25, 27;
  Matrix<double, Dynamic, 1> beta(M, 1);
  beta << 0.3, 2;
  double alpha = 0.3;

  matrix_cl<double> x_cl(x);
  matrix_cl<int> y_cl(y);
  matrix_cl<int> y_vec_cl(y_vec, N, 1);

  expect_near_rel(
      "poisson_log_glm_lpmf (OpenCL)",
      stan::math::poisson_log_glm_lpmf(y_cl, x_cl, alpha, beta),
      stan::math::poisson_log_glm_lpmf(y_vec_cl, x_cl, alpha, beta));
  expect_near_rel(
      "poisson_log_glm_lpmf (OpenCL)",
      stan::math::poisson_log_glm_lpmf<true>(y_cl, x_cl, alpha, beta),
      stan::math::poisson_log_glm_lpmf<true>(y_vec_cl, x_cl, alpha, beta));

  Matrix<var, Dynamic, 1> beta_var1 = beta;
  Matrix<var, Dynamic, 1> beta_var2 = beta;
  var alpha_var1 = alpha;
  var alpha_var2 = alpha;

  var res1
      = stan::math::poisson_log_glm_lpmf(y_cl, x_cl, alpha_var1, beta_var1);
  var res2
      = stan::math::poisson_log_glm_lpmf(y_vec_cl, x_cl, alpha_var2, beta_var2);

  (res1 + res2).grad();

  expect_near_rel("poisson_log_glm_lpmf (OpenCL)", res1.val(), res2.val());

  expect_near_rel("poisson_log_glm_lpmf (OpenCL)", alpha_var1.adj(),
                  alpha_var2.adj());
  expect_near_rel("poisson_log_glm_lpmf (OpenCL)", beta_var1.adj().eval(),
                  beta_var2.adj().eval());
}

TEST(ProbDistributionsPoissonLogGLM, gpu_matches_cpu_zero_instances) {
  double eps = 1e-9;
  int N = 0;
  int M = 2;

  vector<int> y{};
  Matrix<double, Dynamic, Dynamic> x(N, M);
  Matrix<double, Dynamic, 1> beta(M, 1);
  beta << 0.3, 2;
  double alpha = 0.3;

  matrix_cl<double> x_cl(x);
  matrix_cl<int> y_cl(y, N, 1);

  expect_near_rel("poisson_log_glm_lpmf (OpenCL)",
                  stan::math::poisson_log_glm_lpmf(y_cl, x_cl, alpha, beta),
                  stan::math::poisson_log_glm_lpmf(y, x, alpha, beta));
  expect_near_rel(
      "poisson_log_glm_lpmf (OpenCL)",
      stan::math::poisson_log_glm_lpmf<true>(y_cl, x_cl, alpha, beta),
      stan::math::poisson_log_glm_lpmf<true>(y, x, alpha, beta));

  Matrix<var, Dynamic, 1> beta_var1 = beta;
  Matrix<var, Dynamic, 1> beta_var2 = beta;
  var alpha_var1 = alpha;
  var alpha_var2 = alpha;

  var res1
      = stan::math::poisson_log_glm_lpmf(y_cl, x_cl, alpha_var1, beta_var1);
  var res2 = stan::math::poisson_log_glm_lpmf(y, x, alpha_var2, beta_var2);

  (res1 + res2).grad();

  expect_near_rel("poisson_log_glm_lpmf (OpenCL)", res1.val(), res2.val());

  expect_near_rel("poisson_log_glm_lpmf (OpenCL)", alpha_var1.adj(),
                  alpha_var2.adj());
  expect_near_rel("poisson_log_glm_lpmf (OpenCL)", beta_var1.adj().eval(),
                  beta_var2.adj().eval());
}

TEST(ProbDistributionsPoissonLogGLM, gpu_matches_cpu_zero_attributes) {
  double eps = 1e-9;
  int N = 3;
  int M = 0;

  vector<int> y{0, 1, 5};
  Matrix<double, Dynamic, Dynamic> x(N, M);
  Matrix<double, Dynamic, 1> beta(M, 1);
  double alpha = 0.3;

  matrix_cl<double> x_cl(x);
  matrix_cl<int> y_cl(y, N, 1);

  expect_near_rel("poisson_log_glm_lpmf (OpenCL)",
                  stan::math::poisson_log_glm_lpmf(y_cl, x_cl, alpha, beta),
                  stan::math::poisson_log_glm_lpmf(y, x, alpha, beta));
  expect_near_rel(
      "poisson_log_glm_lpmf (OpenCL)",
      stan::math::poisson_log_glm_lpmf<true>(y_cl, x_cl, alpha, beta),
      stan::math::poisson_log_glm_lpmf<true>(y, x, alpha, beta));

  Matrix<var, Dynamic, 1> beta_var1 = beta;
  Matrix<var, Dynamic, 1> beta_var2 = beta;
  var alpha_var1 = alpha;
  var alpha_var2 = alpha;

  var res1
      = stan::math::poisson_log_glm_lpmf(y_cl, x_cl, alpha_var1, beta_var1);
  var res2 = stan::math::poisson_log_glm_lpmf(y, x, alpha_var2, beta_var2);

  (res1 + res2).grad();

  expect_near_rel("poisson_log_glm_lpmf (OpenCL)", res1.val(), res2.val());

  expect_near_rel("poisson_log_glm_lpmf (OpenCL)", alpha_var1.adj(),
                  alpha_var2.adj());
  expect_near_rel("poisson_log_glm_lpmf (OpenCL)", beta_var1.adj().eval(),
                  beta_var2.adj().eval());
}

TEST(ProbDistributionsPoissonLogGLM, gpu_matches_cpu_small_vector_alpha) {
  double eps = 1e-9;
  int N = 3;
  int M = 2;

  vector<int> y{0, 1, 5};
  Matrix<double, Dynamic, Dynamic> x(N, M);
  x << -12, 46, -42, 24, 25, 27;
  Matrix<double, Dynamic, 1> beta(M, 1);
  beta << 0.3, 2;
  Matrix<double, Dynamic, 1> alpha(N, 1);
  alpha << 0.3, -0.8, 1.8;

  matrix_cl<double> x_cl(x);
  matrix_cl<int> y_cl(y, N, 1);

  expect_near_rel("poisson_log_glm_lpmf (OpenCL)",
                  stan::math::poisson_log_glm_lpmf(y_cl, x_cl, alpha, beta),
                  stan::math::poisson_log_glm_lpmf(y, x, alpha, beta));
  expect_near_rel(
      "poisson_log_glm_lpmf (OpenCL)",
      stan::math::poisson_log_glm_lpmf<true>(y_cl, x_cl, alpha, beta),
      stan::math::poisson_log_glm_lpmf<true>(y, x, alpha, beta));

  Matrix<var, Dynamic, 1> beta_var1 = beta;
  Matrix<var, Dynamic, 1> beta_var2 = beta;
  Matrix<var, Dynamic, 1> alpha_var1 = alpha;
  Matrix<var, Dynamic, 1> alpha_var2 = alpha;

  var res1
      = stan::math::poisson_log_glm_lpmf(y_cl, x_cl, alpha_var1, beta_var1);
  var res2 = stan::math::poisson_log_glm_lpmf(y, x, alpha_var2, beta_var2);

  (res1 + res2).grad();

  expect_near_rel("poisson_log_glm_lpmf (OpenCL)", res1.val(), res2.val());
  expect_near_rel("poisson_log_glm_lpmf (OpenCL)", beta_var1.adj().eval(),
                  beta_var2.adj().eval());
  expect_near_rel("poisson_log_glm_lpmf (OpenCL)", alpha_var1.adj().eval(),
                  alpha_var2.adj().eval());
}

TEST(ProbDistributionsPoissonLogGLM, gpu_matches_cpu_big) {
  double eps = 1e-9;
  int N = 153;
  int M = 71;

  vector<int> y(N);
  for (int i = 0; i < N; i++) {
    y[i] = Array<int, Dynamic, 1>::Random(1, 1).abs()(0);
  }
  Matrix<double, Dynamic, Dynamic> x
      = Matrix<double, Dynamic, Dynamic>::Random(N, M);
  Matrix<double, Dynamic, 1> beta = Matrix<double, Dynamic, 1>::Random(M, 1);
  Matrix<double, Dynamic, 1> alpha = Matrix<double, Dynamic, 1>::Random(N, 1);

  matrix_cl<double> x_cl(x);
  matrix_cl<int> y_cl(y, N, 1);

  expect_near_rel("poisson_log_glm_lpmf (OpenCL)",
                  stan::math::poisson_log_glm_lpmf(y_cl, x_cl, alpha, beta),
                  stan::math::poisson_log_glm_lpmf(y, x, alpha, beta));
  expect_near_rel("poisson_log_glm_lpmf (OpenCL)",
                  stan::math::poisson_log_glm_lpmf(y_cl, x_cl, alpha, beta),
                  stan::math::poisson_log_glm_lpmf(y, x, alpha, beta));

  Matrix<var, Dynamic, 1> beta_var1 = beta;
  Matrix<var, Dynamic, 1> beta_var2 = beta;
  Matrix<var, Dynamic, 1> alpha_var1 = alpha;
  Matrix<var, Dynamic, 1> alpha_var2 = alpha;

  var res1 = stan::math::poisson_log_glm_lpmf<true>(y_cl, x_cl, alpha_var1,
                                                    beta_var1);
  var res2
      = stan::math::poisson_log_glm_lpmf<true>(y, x, alpha_var2, beta_var2);

  (res1 + res2).grad();

  expect_near_rel("poisson_log_glm_lpmf (OpenCL)", res1.val(), res2.val());

  expect_near_rel("poisson_log_glm_lpmf (OpenCL)", beta_var1.adj().eval(),
                  beta_var2.adj().eval());
  expect_near_rel("poisson_log_glm_lpmf (OpenCL)", alpha_var1.adj().eval(),
                  alpha_var2.adj().eval());
}

TEST(ProbDistributionsPoissonLogGLM, gpu_matches_cpu_poisson_log_vars_propto) {
  vector<int> y{2, 0, 1, 2, 1, 0, 0, 1, 3, 0};
  Matrix<double, Dynamic, Dynamic> x(10, 3);
  x << -1.87936, 0.55093, -2.50689, 4.78584, 0.988523, -2.46141, 1.46229,
      2.21497, 1.72734, -0.916165, -0.563808, 0.165279, -0.752066, 1.43575,
      0.296557, -0.738422, 0.438686, 0.664492, 0.518203, -0.27485, 2, 0, 1, 2,
      1, 0, 0, 1, 3, 0;
  Matrix<var, Dynamic, 1> beta(3, 1);
  beta << 1.17711, 1.24432, -0.596639;
  var alpha = -1.04272;
  var lp = stan::math::poisson_log_glm_lpmf<true>(
      stan::math::to_matrix_cl(y), stan::math::to_matrix_cl(x), alpha, beta);
  var lp1 = stan::math::poisson_log_glm_lpmf<true>(y, x, alpha, beta);
  EXPECT_FLOAT_EQ(lp.val(), lp1.val());
}

#endif
