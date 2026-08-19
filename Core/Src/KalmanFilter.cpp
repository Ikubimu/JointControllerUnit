#include "KalmanFilter.hpp"

KalmanFilter::KalmanFilter(float pos, float vel, float dt, float ppx, float pvx)
    : dt(dt), ppx(ppx), pvx(pvx),
      s_pos(pos), s_vel(vel),
      s_Qp(KALMAN_QP_DEFAULT), s_Qv(KALMAN_QV_DEFAULT), s_Qpv(KALMAN_QPV_DEFAULT),
      Ppp(0.0f), Ppv(0.0f), Pvv(0.0f)
{
    Ppp = ppx;
    Pvv = pvx;
    Ppv = 0.0f;
}

void KalmanFilter::init(float pos, float vel) {
    s_pos = pos;
    s_vel = vel;
    s_Qp = KALMAN_QP_DEFAULT;
    s_Qv = KALMAN_QV_DEFAULT;
    s_Qpv = KALMAN_QPV_DEFAULT;
    Ppp = ppx;
    Pvv = pvx;
    Ppv = 0.0f;
}

void KalmanFilter::update(float pos_meas, float vel_meas, float vel_model) {
    float pos_pred = s_pos + vel_model * dt;
    float vel_pred = vel_model;

    float Ppp_pred = Ppp + 2.0f * Ppv * dt + Pvv * dt * dt + s_Qp;
    float Pvv_pred = s_Qv;
    float Ppv_pred = Ppv + Pvv * dt + s_Qpv;

    float Kp = Ppp_pred / (Ppp_pred + ppx);
    float Kv = Ppv_pred / (Ppp_pred + ppx);

    Ppp = (1.0f - Kp) * (1.0f - Kp) * Ppp_pred + Kp * Kp * ppx;
    Ppv = (1.0f - Kp) * Ppv_pred - Kp * Kv * ppx;
    Pvv = Pvv_pred - 2.0f * Kv * Ppv_pred + Kv * Kv * (Ppp_pred + ppx);

    s_pos = pos_pred + Kp * (pos_meas - pos_pred);
    s_vel = vel_pred + Kv * (vel_meas - vel_pred);
}

float KalmanFilter::get_position() {
    return s_pos;
}

float KalmanFilter::get_velocity() {
    return s_vel;
}
