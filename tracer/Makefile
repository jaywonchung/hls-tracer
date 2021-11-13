all: control-flow-tracer.ll control-flow-tracer.bc

control-flow-tracer.ll: control-flow-tracer.cpp
	$(CXX) -S -emit-llvm $^ -o $@

control-flow-tracer.bc: control-flow-tracer.cpp
	$(CXX) -c -emit-llvm $^ -o $@

clean:
	rm -f *.ll