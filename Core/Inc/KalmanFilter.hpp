#ifndef KALMAN_FILTER_HPP
#define KALMAN_FILTER_HPP

#define KALMAN_DT_DEFAULT       0.01f
#define KALMAN_PPX_DEFAULT      0.044f
#define KALMAN_PVX_DEFAULT      800.0f
#define KALMAN_QP_DEFAULT       0.0f
#define KALMAN_QV_DEFAULT       0.001f
#define KALMAN_QPV_DEFAULT      0.0f

class KalmanFilter {
public:
    KalmanFilter(float pos, float vel, float dt = KALMAN_DT_DEFAULT,
                 float ppx = KALMAN_PPX_DEFAULT, float pvx = KALMAN_PVX_DEFAULT);

    void    init(float pos, float vel);
    void    update(float pos_meas, float vel_meas, float vel_model);
    float   get_position();
    float   get_velocity();

private:
    float dt;
    float ppx;
    float pvx;

    float s_pos;
    float s_vel;
    float s_Qp;
    float s_Qv;
    float s_Qpv;

    float Ppp;
    float Ppv;
    float Pvv;
};

#endif
