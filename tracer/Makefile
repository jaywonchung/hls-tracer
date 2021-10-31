all: control-flow-tracer.ll

control-flow-tracer.ll: control-flow-tracer.cpp
	$(CXX) -g -S -emit-llvm $^

clean:
	rm -f *.ll