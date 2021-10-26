all: sigma.ll

sigma.ll: sigma.cpp
	$(CXX) -S -emit-llvm $^

clean:
	rm -f *.ll