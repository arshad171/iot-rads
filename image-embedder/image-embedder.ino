// Tensorflow Lite Micro
#include <TensorFlowLite.h>
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

// Tiny ML shield (required for camera)
#include <TinyMLShield.h>

// Our libraries
#include "src/communication/serial/serial.h"
#include "src/utils/logging.h"
#include "src/utils/types.h"
#include "src/utils/common.h"
#include "src/communication/protocol.h"

// Model
#include "working.h"

// Convenience functions
const char* TensorTypeName(TfLiteType type) {
    switch (type) {
        case kTfLiteFloat32: return "Float32";
        case kTfLiteInt32:   return "Int32";
        case kTfLiteUInt8:   return "UInt8";
        case kTfLiteInt8:    return "Int8";
        case kTfLiteInt16:   return "Int16";
        case kTfLiteBool:    return "Bool";
        case kTfLiteFloat16: return "Float16";
        default:             return "Unknown";
    }
}

// Global TFLite objects
namespace {
    // Tensorflow Lite Micro runtime
    const tflite::Model* model = nullptr;
    tflite::MicroInterpreter* interpreter = nullptr;

    // Used for debugging purposes
    tflite::MicroErrorReporter error_reporter;

    // Input/Output tensors
    TfLiteTensor* input = nullptr;
    TfLiteTensor* output = nullptr;

    // Tensor arena (memory region allocated to Tensorflow)
    constexpr int tensor_arena_size = 195 * 1024;
    alignas(16) uint8_t tensor_arena[tensor_arena_size];
}

void setup() {
    // Initialize serial communication
    SP.initialize(19200,1000);
    set_log_lvl(LOG_DEBUG);
    toggle_logs(true);

    SP.blocking_wait(0);
    LOG_SHORT(LOG_INFO,"Serial communication started.");

    // Load the model
    model = tflite::GetModel(model_data);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        LOG(LOG_FATAL,"Model schema version does not match TensorFlow Lite runtime.");
        while(true);
    }

    // Initialize interpreter and operation resolver
    static tflite::AllOpsResolver resolver;
    static tflite::MicroInterpreter static_interpreter(
        model,
        resolver,
        tensor_arena,
        tensor_arena_size,
        &error_reporter
    );

    // Create the global reference to the interpreter
    interpreter = &static_interpreter;

    // Allocate tensors
    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        LOG(LOG_FATAL,"Tensor allocation failed");
        while(true);
    }

    // Obtain pointers to the model's input and output tensors
    input = interpreter->input(0);
    output = interpreter->output(0);

    // Check the type of the input and output
    LOG_SHORT(LOG_INFO,"Input tensor type: %s | Output tensor type: %s",TensorTypeName(input->type),TensorTypeName(output->type));
    LOG_SHORT(LOG_INFO,"Model loaded and tensors allocated correctly.");

    // Initialize the camera
    Camera.begin(QVGA,GRAYSCALE,5,OV7675);
}

void loop() {
    // Take a picture directly in the input tensor
    Camera.readFrame(input->data.int8);

    // Run the model
    if (interpreter->Invoke() == kTfLiteOk) {
        LOG_SHORT(LOG_DEBUG,"Model invoked successfully.");

        // Ensure the output tensor size is as expected
        int output_length = output->bytes/sizeof(float);
        if (output_length == 100) {
            // Construct the output vector as a matrix
            size_t matrix_size = getRichMatrixSize(output_length,1,MatrixType::TYPE_FLOAT32);
            RichMatrix *tensor = (RichMatrix *) memalloc(matrix_size);

            // Fill in the matrix details
            tensor->metadata.rows = output_length;
            tensor->metadata.cols = 1;
            tensor->metadata.type = MatrixType::TYPE_FLOAT32;
            memcpy(tensor->data,output->data.f,output->bytes);

            // Send the output vector
            pack((byte *) tensor,matrix_size,DType::MAT,Cmd::SET_FEATURE_VECTOR,&SP);
            free(tensor);
        } else {
            LOG(LOG_WARNING,"Unexpected output tensor size.");
        }
    } else {
        LOG(LOG_ERROR,"Failed to invoke the interpreter.");
    }

    // Add a delay to avoid continuous inference in the loop
    delay(10000);
}
