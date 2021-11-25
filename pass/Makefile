all: control-flow-trace-pass.so loop-analysis-pass.so

control-flow-trace-pass.so: control-flow-trace-pass.cpp
	$(CXX) $(CXXFLAGS) -shared $^ -o $@ -fPIC $(LDFLAGS)

loop-analysis-pass.so: loop-analysis-pass.cpp
	$(CXX) $(CXXFLAGS) -shared $^ -o $@ -fPIC $(LDFLAGS)

clean:
	rm -f *.o *.so *.ll