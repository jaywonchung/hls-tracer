LLVM_CONFIG=llvm-config
GCC_TOOL_CHAIN_ROOT=$(XILINX_VIVADO)/tps/lnx64/gcc-6.2.0

CXX:=`$(LLVM_CONFIG) --bindir`/clang
CXXFLAGS:=`$(LLVM_CONFIG) --cppflags` -D_DEBUG --gcc-toolchain=$(GCC_TOOL_CHAIN_ROOT) -fPIC -fvisibility-inlines-hidden -Werror=date-time -Werror=unguarded-availability-new -std=c++11 -Wall -Wcast-qual -Wmissing-field-initializers -pedantic -Wno-long-long -Wcovered-switch-default -Wnon-virtual-dtor -Wdelete-non-virtual-dtor -Wstring-conversion -fcolor-diagnostics -ffunction-sections -fdata-sections -O3 -fno-exceptions -fno-rtti -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS
LDFLAGS:=`$(LLVM_CONFIG) --ldflags`

all: control-flow-tracer.so control-flow-tracer.ll

control-flow-tracer.so: control-flow-trace-pass.o
	$(CXX) -shared $^ -o $@ -fPIC $(CXXFLAGS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) -c $^ -o $@ $(CXXFLAGS)

control-flow-tracer.ll: control-flow-tracer.cpp
	$(CXX) -S -emit-llvm $(CXXFLAGS) $^

clean:
	rm -f *.o *.so *.ll