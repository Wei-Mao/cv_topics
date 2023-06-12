def update(mean_pred: float, var_pred: float, mean_measure: float, var_measure: float):
    assert abs(var_pred) > 1e-9 and abs(var_measure) > 1e-9

    new_mean = (var_pred * mean_measure + var_measure * mean_pred) / (var_pred + var_measure)
    new_var = 1.0 / (1.0 / var_pred +  1.0 / var_measure)

    return new_mean, new_var

def predict(mean_curr, var_curr, mean_disp, var_disp):
    '''
        x_{k + 1} = x_{k} + u{k + 1}
    '''
    new_mean = mean_curr + mean_disp
    new_var = var_curr + var_disp

    return new_mean, new_var


if __name__ == '__main__':
    measurements = [5., 6., 7., 9., 10.]
    motion = [1., 1., 2., 1., 1.]
    measurement_sig = 4.
    motion_sig = 2.

    # starting state
    mu = 0.
    sig = 10000.
    sig = 0.000000000011

    for k in range(len(measurements)):
        mu, sig = update(mu, sig, measurements[k], measurement_sig)
        print(f"update:  [{mu}, {sig}]")

        mu, sig = predict(mu, sig, motion[k], motion_sig)
        print(f"predict: [{mu}, {sig}]")
