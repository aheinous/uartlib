.DELETE_ON_ERROR:

TEMPLATE := test_template.mk


ALL_TESTS := ezp_msg_buffer_test ezp_byte_buffer_test ezp_csum_test ezp_reader_sender_test ezp_master_test
ALL_RUNS := $(ALL_TESTS:%=run-%)

# EXTRA_SRCS := fakes/ezp_platform_stubs.c
# export EXTRA_SRCS

.PHONY: all run-all clean

all: $(ALL_TESTS)

run-all: $(ALL_RUNS)

run-%: %
	./build/$</$<

ezp_msg_buffer_test:
	@TEST_NAME=$@ MSG_TABLE=foobarpingpong.h $(MAKE) -e -f $(TEMPLATE)

ezp_byte_buffer_test:
	@TEST_NAME=$@ MSG_TABLE=foobarpingpong.h  $(MAKE) -e -f $(TEMPLATE)

ezp_csum_test:
	@TEST_NAME=$@ MSG_TABLE=foobarpingpong.h $(MAKE) -e -f $(TEMPLATE)

ezp_master_test:
	@TEST_NAME=$@ MSG_TABLE=foobarpingpong.h $(MAKE) -e -f $(TEMPLATE)


ezp_reader_sender_test:
	@TEST_NAME=$@ MSG_TABLE=foobarpingpong.h  EXTRA_SRCS=fakes/ezp_byte_buffer.c $(MAKE) -e -f $(TEMPLATE)



clean:
	@rm -r ./build/