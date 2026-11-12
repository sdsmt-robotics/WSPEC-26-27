#include <cmath>
#include <algorithm>

class SensorFusion1D {
private:
    // State Vector: [p, v]
    double p = 0.0; // Fused position (meters)
    double v = 0.0; // Fused velocity (m/s)

    // Covariance Matrix P (2x2 symmetric)
    double P11 = 1.0;
    double P12 = 0.0;
    double P21 = 0.0;
    double P22 = 1.0;

    // Process noise spectral densities
    double Q_accel = 0.5; // Expected variance in physical acceleration (m/s^2)^2

public:
    SensorFusion1D(double initial_p = 0.0, double initial_v = 0.0)
        : p(initial_p), v(initial_v) {}

    // Time Update (Predict step)
    void predict(double dt) {
        if (dt <= 0.0) return;

        // Kinematic state extrapolation: p = p + v * dt
        p += v * dt;

        // Process noise matrix Q driven by acceleration uncertainty
        double dt2 = dt * dt;
        double dt3 = dt2 * dt;
        double dt4 = dt2 * dt2;

        double Q11 = 0.25 * dt4 * Q_accel;
        double Q12 = 0.50 * dt3 * Q_accel;
        double Q22 = dt2 * Q_accel;

        // Covariance update: P = F * P * F^T + Q
        double P11_new = P11 + dt * (P12 + P21) + dt2 * P22 + Q11;
        double P12_new = P12 + dt * P22 + Q12;
        double P22_new = P22 + Q22;

        P11 = P11_new;
        P12 = P12_new;
        P21 = P12_new;
        P22 = P22_new;
    }

    // Scalar Measurement Update (Correct step for a single velocity sensor)
    void update_velocity(double v_meas, double var_meas) {
        // Measurement matrix H = [0, 1]
        // Innovation residual
        double y = v_meas - v;

        // Residual covariance S = H * P * H^T + R
        double S = P22 + var_meas;

        // Optimal Kalman Gain K = P * H^T / S
        double K1 = P12 / S;
        double K2 = P22 / S;

        // State update
        p += K1 * y;
        v += K2 * y;

        // Covariance update: P = (I - K * H) * P
        double P11_new = P11 - K1 * P12;
        double P12_new = P12 - K1 * P22;
        double P22_new = P22 - K2 * P22;

        P11 = P11_new;
        P12 = P12_new;
        P21 = P12_new;
        P22 = P22_new;
    }

    // Getters
    double get_position() const { return p; }
    double get_velocity() const { return v; }
};

//A potential helper function to convert mouse counts to velocity in meters per second, 
//given the effective counts per inch (CPI) and the time delta in seconds.
// This function is commented out for now, but can be used if needed for sensor fusion calculations.
//double mouse_counts_to_velocity(int16_t delta_counts, double cpi_effective, double dt) {
//    if (dt <= 0.0001) return 0.0;
//    constexpr double INCHES_TO_METERS = 0.0254;
//    double delta_meters = (static_cast<double>(delta_counts) / cpi_effective) * INCHES_TO_METERS;
//    return delta_meters / dt;
//}