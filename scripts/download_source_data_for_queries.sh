echo "Downloading source data for queries..."
wget https://github.com/PhamHung2020/Simple-SGX-based-DSP-Engine/releases/download/data_source/source_data.zip -O ./source_data.zip
echo "Extracting source data..."
unzip ./source_data.zip -d ./source_data
rm ./source_data.zip
echo "Source data downloaded and extracted to ./source_data"