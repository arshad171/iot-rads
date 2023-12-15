#pragma once
#include "../../settings/dnn.h"
#include <BasicLinearAlgebra.h>

// NN Layers
#include "layers/linear.h"
#include "layers/relu.h"
#include "losses/squared.h"

#include "../utils/types.h"

struct NetworkShape {
    LinearLayer<FEATURE_DIM, 10, BATCH_SIZE> lin1;
    ReLULayer<10, BATCH_SIZE> rel1;

    LinearLayer<10, 2, BATCH_SIZE> lin2;
    ReLULayer<2, BATCH_SIZE> rel2;

    LinearLayer<2, 10, BATCH_SIZE> lin3;
    ReLULayer<10, BATCH_SIZE> rel3;

    LinearLayer<10, FEATURE_DIM, BATCH_SIZE> lin4;
    ReLULayer<FEATURE_DIM, BATCH_SIZE> rel4;
};

extern BLA::Matrix<FEATURE_DIM, BATCH_SIZE> xBatch;
extern NetworkShape network;

void initialize_network();
void begin_training();
bool process_feature(RichMatrix *vector);
float get_training_loss();
void blocking_train();
void send_layer_weights(uint16_t layerIndex);

float predict(BLA::Matrix<FEATURE_DIM, BATCH_SIZE> x);
