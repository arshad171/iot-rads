# Railway Anomaly Detection System (RADS)

## Proof of Concept

The POC is run on opensource dataset. Here are the steps:

1. Gather data, run `dump_data/dump_open_data.py`.

2. Parse the data, run `dump_data/parse_open_data.py`.

3. Generating mask, run the `test_resize.ipynb` to dump the resized image (suitable for the embeddng model). The mask (ROI) can then be manually generated using MATLAB segmenter and export the binary mask with the variable name `mask` and the filename `mask.mat`. Make sure to normalize the binary mask before exporting, i.e. divide it by 255.0.

4. The [interactive notebook](./test_anomaly_detection.ipynb) trains the autoencoder and vizualizes the results on sample images.

Note: **For each of the scipts above make sure to checkout the config variables defined at the top of the scripts. These can be used to change the source/dest paths and other parameters**