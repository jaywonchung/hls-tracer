all: control-flow-tracer.ll control-flow-tracer.bc

control-flow-tracer.ll: control-flow-tracer.c
	$(CXX) -S -emit-llvm $^ -o $@

control-flow-tracer.bc: control-flow-tracer.c
	$(CXX) -c -emit-llvm $^ -o $@

clean:
	rm -f *.ll *.bc