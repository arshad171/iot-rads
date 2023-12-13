#include "../../../settings/dnn.h"
#include "../../utils/logging.h"

// NN Layers
#include "../network.h"

#include "central.h"

namespace CENTRAL {
    bool biasFlag = false;
    int layerIndex = 1;
    int rowIndex, colIndex = 0;
    float avgWeightsBuffer[BLE_NUM_WEIGHTS] = {0.0};

    WeightsBuffer sendWeightsBuffer;

    int trainflag = 0;
    bool stopFlag = false;
    bool value = false;
    BLEDevice peripheral;
    BLECharacteristic writeCharacteristic;
    BLECharacteristic readCharacteristic;
    WeightsBuffer receiveWeightsBuffer;

    void BLEserviceConnect()
    {

      if (peripheral)
      {
        // discovered a peripheral, print out address, local name, and advertised service
        // LOG_SHORT(LOG_INFO, "BLE::found peripheral: %s %s", peripheral.address(), peripheral.localName());
        LOG_SHORT(LOG_INFO, "BLE::found peripheral");

        if (peripheral.localName() != "sender")
        {
          return;
        }

        // stop scanning
        BLE.stopScan();

        // controlLed(peripheral);
        // peripheral.connect();
        if (peripheral.connect())
        {
          LOG_SHORT(LOG_INFO, "BLE::connected");
        }
        else
        {
          LOG_SHORT(LOG_ERROR, "BLE::disconnected");
          return;
        }
        // peripheral.discoverAttributes();
        //  discover peripheral attributes
        LOG_SHORT(LOG_INFO, "BLE::discovering attributes");
        if (peripheral.discoverAttributes())
        {
          LOG_SHORT(LOG_INFO, "BLE::attributes discovered");
        }
        else
        {
          LOG_SHORT(LOG_ERROR, "BLE::attributes discovery");
          peripheral.disconnect();
          return;
        }
      }
    }

    void sendBiasBatch()
    {
      // sendWeightsBuffer.syncFlag = false;
      // sendWeightsBuffer.biasFlag = true;
      // sendWeightsBuffer.layerIndex = layerIndex;
      // sendWeightsBuffer.rowIndex = rowIndex;
      // sendWeightsBuffer.colIndex = colIndex;

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

      writeCharacteristic.writeValue((byte *)&sendWeightsBuffer, sizeof(sendWeightsBuffer));

      LOG_SHORT(LOG_INFO, "BLE::sent bias packet");
      LOG_SHORT(LOG_INFO, "layer: %d | rowIndex: %d | colIndex: %d | biasFlag: %d | stopFlag: %d", layerIndex, rowIndex, colIndex, biasFlag, stopFlag);
    }

    void receiveBiasBatch()
    {
      if (!receiveWeightsBuffer.biasFlag)
        return;

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

    void receiveBiasBatchAvg()
    {
      if (!receiveWeightsBuffer.biasFlag)
        return;

      switch (layerIndex)
      {
      case 1:
        network.lin1.copyBiasToBuffer(colIndex, avgWeightsBuffer, BLE_NUM_WEIGHTS);
        break;
      case 2:
        network.lin2.copyBiasToBuffer(colIndex, avgWeightsBuffer, BLE_NUM_WEIGHTS);
        break;
      case 3:
        network.lin3.copyBiasToBuffer(colIndex, avgWeightsBuffer, BLE_NUM_WEIGHTS);
        break;
      case 4:
        network.lin4.copyBiasToBuffer(colIndex, avgWeightsBuffer, BLE_NUM_WEIGHTS);
        break;
      }

      for (int i = 0; i < BLE_NUM_WEIGHTS; i++)
      {
        receiveWeightsBuffer.weights[i] = 0.5 * (receiveWeightsBuffer.weights[i] + avgWeightsBuffer[i]);
      }

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

    void sendWeightsBatch()
    {
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

      writeCharacteristic.writeValue((byte *)&sendWeightsBuffer, sizeof(sendWeightsBuffer));
      LOG_SHORT(LOG_INFO, "BLE::sent weights packet");
      LOG_SHORT(LOG_INFO, "layer: %d | rowIndex: %d | colIndex: %d | biasFlag: %d | stopFlag: %d", layerIndex, rowIndex, colIndex, biasFlag, stopFlag);
    }

    void receiveWeightsBatch()
    {
      if (receiveWeightsBuffer.biasFlag)
        return;

      if (receiveWeightsBuffer.syncFlag == true)
      {
        LOG_SHORT(LOG_DEBUG, "BLE::not synchronized");
        return;
      }

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

    void receiveWeightsBatchAvg()
    {
      if (receiveWeightsBuffer.biasFlag)
        return;

      if (receiveWeightsBuffer.syncFlag == true)
      {
        LOG_SHORT(LOG_DEBUG, "BLE::not synchronized");
        return;
      }

      switch (layerIndex)
      {
      case 1:
        network.lin1.copyWeightsToBuffer(rowIndex, colIndex, avgWeightsBuffer, BLE_NUM_WEIGHTS);
        break;
      case 2:
        network.lin2.copyWeightsToBuffer(rowIndex, colIndex, avgWeightsBuffer, BLE_NUM_WEIGHTS);
        break;
      case 3:
        network.lin3.copyWeightsToBuffer(rowIndex, colIndex, avgWeightsBuffer, BLE_NUM_WEIGHTS);
        break;
      case 4:
        network.lin4.copyWeightsToBuffer(rowIndex, colIndex, avgWeightsBuffer, BLE_NUM_WEIGHTS);
        break;
      }

      for (int i = 0; i < BLE_NUM_WEIGHTS; i++)
      {
        receiveWeightsBuffer.weights[i] = 0.5 * (receiveWeightsBuffer.weights[i] + avgWeightsBuffer[i]);
      }

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

    void send()
    {
      sendWeightsBuffer.stopFlag = stopFlag;
      sendWeightsBuffer.biasFlag = biasFlag;
      sendWeightsBuffer.layerIndex = layerIndex;
      sendWeightsBuffer.rowIndex = rowIndex;
      sendWeightsBuffer.colIndex = colIndex;
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
      readCharacteristic.readValue((byte *)&receiveWeightsBuffer, sizeof(receiveWeightsBuffer));

      stopFlag = receiveWeightsBuffer.stopFlag;
      biasFlag = receiveWeightsBuffer.biasFlag;
      layerIndex = receiveWeightsBuffer.layerIndex;
      rowIndex = receiveWeightsBuffer.rowIndex;
      colIndex = receiveWeightsBuffer.colIndex;

      if (!biasFlag)
      {
        receiveWeightsBatchAvg();
      }
      else
      {
        receiveBiasBatchAvg();
      }
    }

    void central_setup()
    {
      // BLE initialization
      // begin initialization
      // Serial.begin(9600);
      BLE.begin();

      // start scanning for peripherals
      BLE.scanForUuid("19b10011-e8f2-537e-4f6c-d104768a1214");

      LOG_SHORT(LOG_INFO, "BLE::central ready");
    }
}