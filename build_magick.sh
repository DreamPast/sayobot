apt-get install libfreetype-dev libpng-dev libjpeg-dev -y
cd ImageMagick
./configure --without-xml
make
make install
