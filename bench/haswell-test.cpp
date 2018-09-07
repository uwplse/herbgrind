#include <cstdlib>
#include <iostream>

#include <Eigen/Core>
#include <Eigen/Cholesky>

#include <herbgrind.h>

int main(int argc __attribute__((unused)),
         char **argv __attribute__((unused))) {
  // Make sure we get consistent results from the RNG
  std::srand(22);

  // This succeeds with a 2x2 matrix and fails with a 3x3 matrix.
  const Eigen::MatrixXd A(Eigen::MatrixXd::Random(3, 3));
  const Eigen::MatrixXd ATA(A.transpose() * A);
  HERBGRIND_BEGIN();
  const Eigen::LDLT<Eigen::MatrixXd> Achol(ATA);
  HERBGRIND_END();

  std::cout << "D: " << Achol.vectorD().transpose() << std::endl;
  std::cout << "L:" << std::endl
            << Eigen::MatrixXd(Achol.matrixL()) << std::endl;

  return 0;
}
