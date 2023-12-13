#include "../../../settings/dnn.h"
#include "../../utils/common.h"
#include "peripheral.h"

// NN Layers
#include "../network.h"

namespace PERIPHERAL {
    const int BLE_NUM_WEIGHTS = 25;

    typedef struct __attribute__((packed))
    {
      bool syncFlag = false;
      bool biasFlag = false;
      bool stopFlag = false;
      int layerIndex;
      int rowIndex;
      int colIndex;
      float weights[BLE_NUM_WEIGHTS];
    } WeightsBuffer;

    const int inputLayersSizes[] = {FEATURE_DIM, 10, 2, 10};
    const int outputLayersSizes[] = {10, 2, 10, FEATURE_DIM};

    bool biasFlag = false;

    int layerIndex = 1;
    int rowIndex, colIndex = 0;
    float avgWeightsBuffer[BLE_NUM_WEIGHTS] = {0.0};

    WeightsBuffer sendWeightsBuffer, receiveWeightsBuffer;

    BLEService networkService("19B10011-E8F2-537E-4F6C-D104768A1214"); // Bluetooth® Low Energy LED Service

    BLECharacteristic readCharacteristic("2A36", BLERead | BLEIndicate, sizeof(sendWeightsBuffer));

    bool sendFlag = true;
    bool stopFlag = false;
    BLECharacteristic writeCharacteristic("2A37", BLEWrite, sizeof(sendWeightsBuffer));

    void blePeripheralConnectHandler(BLEDevice central)
    {
      // central connected event handler
      LOG_SHORT(LOG_INFO, "BLE::connected event central: %s", central.address());
      BLE.advertise();
    }

    void blePeripheralDisconnectHandler(BLEDevice central)
    {
      // central disconnected event handler
      LOG_SHORT(LOG_INFO, "BLE::disconnected event central: %s", central.address());
      BLE.advertise();
      sendFlag = true;
    }

    void sendSyncFlag()
    {
      sendWeightsBuffer.syncFlag = true;
      sendWeightsBuffer.layerIndex = 1;
      sendWeightsBuffer.rowIndex = 0;
      sendWeightsBuffer.colIndex = 0;
      for (int i = 0; i < BLE_NUM_WEIGHTS; i++)
      {
        sendWeightsBuffer.weights[i] = 0.0;
      }
      readCharacteristic.writeValue((byte *)&sendWeightsBuffer, sizeof(sendWeightsBuffer));
      LOG_SHORT(LOG_DEBUG, "BLE::sending sync packet");
    }

    void sendWeightsBatch()
    {
      sendWeightsBuffer.syncFlag = false;
      sendWeightsBuffer.biasFlag = false;

      switch (layerIndex)
      {
      case 1:
        network.lin1.copyWeightsToBuffer(rowIndex, colIndex, sendWeightsBuffer.weights, BLE_NUM_WEIGHTS);
        break;
      case 2:
        network.lin2.copyWeightsToBuffer(rowIndex, colIndex, sendWeightsBuffer.weights, BLE_NUM_WEIGHTS);
        break;
      case 3:
        network.lin3.copyWeightsToBuffer(rowIndex, colIndex, sendWeightsBuffer.weights, BLE_NUM_WEIGHTS);
        break;
      case 4:
        network.lin4.copyWeightsToBuffer(rowIndex, colIndex, sendWeightsBuffer.weights, BLE_NUM_WEIGHTS);
        break;
      }

      readCharacteristic.writeValue((byte *)&sendWeightsBuffer, sizeof(sendWeightsBuffer));
      LOG_SHORT(LOG_INFO, "BLE::sent weights packet");
      LOG_SHORT(LOG_INFO, "layer: %d | rowIndex: %d | colIndex: %d | biasFlag: %d | stopFlag: %d", layerIndex, rowIndex, colIndex, biasFlag, stopFlag);
    }

    void receiveWeightsBatch()
    {
      if (receiveWeightsBuffer.biasFlag)
        return;

      int layerIndex = receiveWeightsBuffer.layerIndex;
      int rowIndex = receiveWeightsBuffer.rowIndex;
      int colIndex = receiveWeightsBuffer.colIndex;
      bool biasFlag = receiveWeightsBuffer.biasFlag;

      switch (layerIndex)
      {
      case 1:
        network.lin1.copyWeightsFromBuffer(rowIndex, colIndex, receiveWeightsBuffer.weights, BLE_NUM_WEIGHTS);
        break;
      case 2:
        network.lin2.copyWeightsFromBuffer(rowIndex, colIndex, receiveWeightsBuffer.weights, BLE_NUM_WEIGHTS);
        break;
      case 3:
        network.lin3.copyWeightsFromBuffer(rowIndex, colIndex, receiveWeightsBuffer.weights, BLE_NUM_WEIGHTS);
        break;
      case 4:
        network.lin4.copyWeightsFromBuffer(rowIndex, colIndex, receiveWeightsBuffer.weights, BLE_NUM_WEIGHTS);
        break;
      }

      LOG_SHORT(LOG_INFO, "BLE::received weights packet");
      LOG_SHORT(LOG_INFO, "layer: %d | rowIndex: %d | colIndex: %d | biasFlag: %d | stopFlag: %d", layerIndex, rowIndex, colIndex, biasFlag, stopFlag);
    }

    void sendBiasBatch()
    {
      sendWeightsBuffer.syncFlag = false;
      sendWeightsBuffer.biasFlag = true;

      switch (layerIndex)
      {
      case 1:
        network.lin1.copyBiasToBuffer(colIndex, sendWeightsBuffer.weights, BLE_NUM_WEIGHTS);
        break;
      case 2:
        network.lin2.copyBiasToBuffer(colIndex, sendWeightsBuffer.weights, BLE_NUM_WEIGHTS);
        break;
      case 3:
        network.lin3.copyBiasToBuffer(colIndex, sendWeightsBuffer.weights, BLE_NUM_WEIGHTS);
        break;
      case 4:
        network.lin4.copyBiasToBuffer(colIndex, sendWeightsBuffer.weights, BLE_NUM_WEIGHTS);
        break;
      }

      readCharacteristic.writeValue((byte *)&sendWeightsBuffer, sizeof(sendWeightsBuffer));
      LOG_SHORT(LOG_INFO, "BLE::sent bias packet");
      LOG_SHORT(LOG_INFO, "layer: %d | rowIndex: %d | colIndex: %d | biasFlag: %d | stopFlag: %d", layerIndex, rowIndex, colIndex, biasFlag, stopFlag);
    }

    void receiveBiasBatch()
    {
      if (!receiveWeightsBuffer.biasFlag)
        return;

      int layerIndex = receiveWeightsBuffer.layerIndex;
      int rowIndex = receiveWeightsBuffer.rowIndex;
      int colIndex = receiveWeightsBuffer.colIndex;
      bool biasFlag = receiveWeightsBuffer.biasFlag;

      switch (layerIndex)
      {
      case 1:
        network.lin1.copyBiasFromBuffer(colIndex, receiveWeightsBuffer.weights, BLE_NUM_WEIGHTS);
        break;
      case 2:
        network.lin2.copyBiasFromBuffer(colIndex, receiveWeightsBuffer.weights, BLE_NUM_WEIGHTS);
        break;
      case 3:
        network.lin3.copyBiasFromBuffer(colIndex, receiveWeightsBuffer.weights, BLE_NUM_WEIGHTS);
        break;
      case 4:
        network.lin4.copyBiasFromBuffer(colIndex, receiveWeightsBuffer.weights, BLE_NUM_WEIGHTS);
        break;
      }

      LOG_SHORT(LOG_INFO, "BLE::received bias packet");
      LOG_SHORT(LOG_INFO, "layer: %d | rowIndex: %d | colIndex: %d | biasFlag: %d | stopFlag: %d", layerIndex, rowIndex, colIndex, biasFlag, stopFlag);
    }

    void send()
    {
      sendWeightsBuffer.layerIndex = layerIndex;
      sendWeightsBuffer.rowIndex = rowIndex;
      sendWeightsBuffer.colIndex = colIndex;
      sendWeightsBuffer.biasFlag = biasFlag;
      sendWeightsBuffer.stopFlag = stopFlag;

      if (!biasFlag)
      {
        sendWeightsBatch();
      }
      else
      {
        sendBiasBatch();
      }
    }

    void receive()
    {
      writeCharacteristic.readValue((byte *)&receiveWeightsBuffer, sizeof(receiveWeightsBuffer));

      if (!biasFlag)
      {
        receiveWeightsBatch();
      }
      else
      {
        receiveBiasBatch();
      }

      layerIndex = receiveWeightsBuffer.layerIndex;
      rowIndex = receiveWeightsBuffer.rowIndex;
      colIndex = receiveWeightsBuffer.colIndex;
      biasFlag = receiveWeightsBuffer.biasFlag;
      // ignore the stop flag received (causes a problem)
      // stopFlag = receiveWeightsBuffer.stopFlag;

      if (!biasFlag)
      {
        colIndex += BLE_NUM_WEIGHTS;

        if (colIndex >= inputLayersSizes[layerIndex - 1])
        {
          // move on to next neuron
          rowIndex += 1;
          colIndex = 0;
        }

        if (rowIndex >= outputLayersSizes[layerIndex - 1])
        {
          // move to next layer
          rowIndex = 0;
          colIndex = 0;
          layerIndex += 1;

          // switch to sending bias
          if (layerIndex >= 5)
          {
            layerIndex = 1;
            rowIndex = 0;
            colIndex = 0;

            biasFlag = true;
          }
        }
      }
      else
      {
        colIndex += BLE_NUM_WEIGHTS;
        if (colIndex >= outputLayersSizes[layerIndex - 1])
        {
          // next layer, start at colIndex = 0
          layerIndex += 1;
          colIndex = 0;

          // swtich back to sending weights
          if (layerIndex >= 5)
          {
            layerIndex = 1;
            rowIndex = 0;
            colIndex = 0;

            biasFlag = false;
            stopFlag = true;
            sendFlag = true;
          }
        }
      }
    }

    void setup_peripheral() {
      // BLE initialization
      // begin initialization
      if (!BLE.begin())
      {
        LOG_SHORT(LOG_ERROR, "BLE::init failed");

        while (1)
          ;
      }

      // set advertised local name and service UUID:
      BLE.setLocalName("sender");
      BLE.setAdvertisedService(networkService);

      // add the characteristic to the service
      networkService.addCharacteristic(readCharacteristic);
      networkService.addCharacteristic(writeCharacteristic);

      // add service
      BLE.addService(networkService);

      BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
      BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

      // set the initial value for the characeristic:
      sendSyncFlag();

      // start advertising
      BLE.advertise();

      LOG_SHORT(LOG_INFO, "BLE::peripheral ready");
    }
}