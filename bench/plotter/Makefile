all: good.bmp bad.bmp

EasyBMP.zip:
	wget 'https://downloads.sourceforge.net/project/easybmp/easybmp/EasyBMP%20Library%20Files%20Version%201.06/EasyBMP_1.06.zip' -O $@

EasyBMP: EasyBMP.zip
	mkdir $@
	unzip -d $@ $^ 

EasyBMP/EasyBMP.o: EasyBMP
	$(CXX) -O3 EasyBMP/EasyBMP.cpp -c -o $@

plotter.std: main.cpp EasyBMP/EasyBMP.o
	$(CXX) -O3 -g $^ -DSQRT=sqrt -o $@

plotter.good: main.cpp EasyBMP/EasyBMP.o
	$(CXX) -O3 -g $^ -DSQRT=goodsqrt -o $@

plotter.bad: main.cpp EasyBMP/EasyBMP.o
	$(CXX) -O3 -g $^ -DSQRT=badsqrt -o $@

%.bmp: plotter.%
	./$< 1000 $@

clean:
	rm -f EasyBMP.zip
	rm -fr EasyBMP
	rm -f plotter.*
	rm -f *.bmp
