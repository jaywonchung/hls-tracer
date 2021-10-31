all: sigma.ll

sigma.ll: sigma.cpp
	$(CXX) -g -S -emit-llvm $^

clean:
	rm -f *.ll