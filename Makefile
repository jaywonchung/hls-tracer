LLVM_CONFIG=llvm-config
GCC_TOOL_CHAIN_ROOT=$(XILINX_VIVADO)/tps/lnx64/gcc-6.2.0

CXX:=`$(LLVM_CONFIG) --bindir`/clang
CXXFLAGS:=`$(LLVM_CONFIG) --cxxflags` -D_DEBUG --gcc-toolchain=$(GCC_TOOL_CHAIN_ROOT) -fPIC -fvisibility-inlines-hidden -Werror=date-time -Werror=unguarded-availability-new -std=c++11 -Wall -Wcast-qual -Wmissing-field-initializers -pedantic -Wno-long-long -Wcovered-switch-default -Wnon-virtual-dtor -Wdelete-non-virtual-dtor -Wstring-conversion -fcolor-diagnostics -ffunction-sections -fdata-sections -O0 -g -fno-exceptions -fno-rtti -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS
LDFLAGS:=`$(LLVM_CONFIG) --ldflags`

SUBDIRS:=pass tracer testfunctions
all:
	for dir in $(SUBDIRS); do \
		CXX="$(CXX)" CXXFLAGS="$(CXXFLAGS)" LDFLAGS="$(LDFLAGS)" make -C $$dir; \
	done
	llvm-link tracer/*.ll testfunctions/*.ll -o out.ll
	llvm-dis -o out.ll out.ll

clean:
	rm -f *.ll
	for dir in $(SUBDIRS); do \
		make clean -C $$dir; \
	done