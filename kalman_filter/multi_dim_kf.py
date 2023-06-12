# Write a function 'kalman_filter' that implements a multi-
# dimensional Kalman Filter for the example given

import math

class Matrix:
    # implements basic operations of a matrix class
    def __init__(self, value: list[list]):
        self.value: list[list]         = value
        self.dimx = len(value)
        self.dimy = len(value[0])
        if value == [[]]:
            self.dimx = 0

    def zero(self, dimx, dimy):
        # check if valid dimensions
        if dimx < 1 or dimy < 1:
            raise ValueError("Invalid size of matrix")
        else:
            self.dimx = dimx
            self.dimy = dimy
            self.value = [[0 for row in range(dimy)] for col in range(dimx)]

    def identity(self, dim):
        # check if valid dimension
        if dim < 1:
            raise ValueError("Invalid size of matrix")
        else:
            self.dimx = dim
            self.dimy = dim
            self.value = [[0 for row in range(dim)] for col in range(dim)]
            for i in range(dim):
                self.value[i][i] = 1

    def show(self):
        for i in range(self.dimx):
            print(self.value[i])
        print(' ')

    def __add__(self, other):
        # check if correct dimensions
        if self.dimx != other.dimx or self.dimy != other.dimy:
            raise ValueError("Matrices must be of equal dimensions to add")
        else:
            # add if correct dimensions
            res = Matrix([[]])
            res.zero(self.dimx, self.dimy)
            for i in range(self.dimx):
                for j in range(self.dimy):
                    res.value[i][j] = self.value[i][j] + other.value[i][j]
            return res

    def __sub__(self, other):
        # check if correct dimensions
        if self.dimx != other.dimx or self.dimy != other.dimy:
            raise ValueError("Matrices must be of equal dimensions to subtract")
        else:
            # subtract if correct dimensions
            res = Matrix([[]])
            res.zero(self.dimx, self.dimy)
            for i in range(self.dimx):
                for j in range(self.dimy):
                    res.value[i][j] = self.value[i][j] - other.value[i][j]
            return res

    def __mul__(self, other):
        # check if correct dimensions
        if self.dimy != other.dimx:
            raise ValueError("Matrices must be m*n and n*p to multiply")
        else:
            # multiply if correct dimensions
            res = Matrix([[]])
            res.zero(self.dimx, other.dimy)
            for i in range(self.dimx):
                for j in range(other.dimy):
                    for k in range(self.dimy):
                        res.value[i][j] += self.value[i][k] * other.value[k][j]
            return res

    def transpose(self):
        # compute transpose
        res = Matrix([[]])
        res.zero(self.dimy, self.dimx)
        for i in range(self.dimx):
            for j in range(self.dimy):
                res.value[j][i] = self.value[i][j]
        return res

    # Thanks to Ernesto P. Adorio for use of Cholesky and CholeskyInverse functions
    def Cholesky(self, ztol=1.0e-5):
        # Computes the upper triangular Cholesky factorization of
        # a positive definite matrix.
        res = Matrix([[]])
        res.zero(self.dimx, self.dimx)

        for i in range(self.dimx):
            S = sum([(res.value[k][i])**2 for k in range(i)])
            d = self.value[i][i] - S
            if abs(d) < ztol:
                res.value[i][i] = 0.0
            else:
                if d < 0.0:
                    raise ValueError("Matrix not positive-definite")
                res.value[i][i] = math.sqrt(d)
            for j in range(i+1, self.dimx):
                S = sum([res.value[k][i] * res.value[k][j] for k in range(self.dimx)])
                if abs(S) < ztol:
                    S = 0.0
                try:
                   res.value[i][j] = (self.value[i][j] - S)/res.value[i][i]
                except:
                   raise ValueError("Zero diagonal")
        return res

    def CholeskyInverse(self):
        # ref: http://www.seas.ucla.edu/~vandenbe/133A/lectures/chol.pdf
        # https://ww2.mathworks.cn/matlabcentral/answers/303431-matrix-inverse-using-cholesky-decomposition
        # Computes inverse of matrix given its Cholesky upper Triangular
        # decomposition of matrix.
        res = Matrix([[]])
        res.zero(self.dimx, self.dimx)

        # Backward step for inverse.
        for j in reversed(range(self.dimx)):
            tjj = self.value[j][j]
            S = sum([self.value[j][k]*res.value[j][k] for k in range(j+1, self.dimx)])
            res.value[j][j] = 1.0/tjj**2 - S/tjj
            for i in reversed(range(j)):
                res.value[j][i] = res.value[i][j] = -sum([self.value[i][k]*res.value[k][j] for k in range(i+1, self.dimx)])/self.value[i][i]
        return res

    def inverse(self):
        aux = self.Cholesky()
        res = aux.CholeskyInverse()
        return res

    # overload __repr__
    def __repr__(self):
        return repr(self.value)


########################################
# Implement the filter function below
#  State transition model: s_{t + 1} = A_{t} s_{t} + u_{t} + epsilon_{t} with epsilon_{t} ~ N(0, Q_{t})
#  Measurement model: z_{t} = H_{t} * s_{t} + delta_{t} with delta ~ N(0, R_{t})
# What we predict and update are the mean and covariance of state vector which is a
# multi-dimensional Gaussian model induced by the white noise.
########################################

def kalman_filter(x: Matrix, sig: Matrix, measurements: list,
        A: Matrix, u: Matrix,  H: Matrix, Q: Matrix, R: Matrix):
    '''
    '''
    I: Matrix = Matrix(value=[[]])
    I.identity(sig.dimx)

    for n in range(len(measurements)):
        # measurement update
        # given: mean_pred, sig_pred, measurement
        # determine: mean_updated, covar_updated
        S: Matrix = H * sig * H.transpose() + R
        K: Matrix = sig * H.transpose() * S.inverse()
        z = Matrix([[measurements[n]]])
        x = x + (K * (z - (H * x)))  # note the parentheses
        sig = (I - (K * H)) * sig

        # prediction step
        # given: mean and covariance from measurement update step
        # For the first iteration
        # determine: mean_pred, covar_pred
        x = (A * x) + u
        sig = A * sig * A.transpose() + Q

        print(f'x: {x}')
        print(f'P: {sig}')

    return x, sig

############################################
### use the code below to test your filter!
############################################

measurements = [1, 2, 3]

x = Matrix([[0.], [0.]]) # initial state (location and velocity)
sig = Matrix([[1000., 0.], [0., 1000.]]) # initial uncertainty

u = Matrix([[0.], [0.]]) # external motion
A = Matrix([[1., 1.], [0, 1.]]) # next state function
Q = Matrix([[10., 0.], [0., 10.]])

H = Matrix([[1., 0.]]) # measurement function
R = Matrix([[1.]]) # measurement uncertainty

x_final, sig_final = kalman_filter(x=x, sig=sig, measurements=measurements, A=A, u=u, H=H, Q=Q, R=R)

# print(kalman_filter(x, P))
# output should be:
# x: [[3.9996664447958645], [0.9999998335552873]]
# P: [[2.3318904241194827, 0.9991676099921091], [0.9991676099921067, 0.49950058263974184]]
