TEMPLATE := test_template.mk


ALL_TESTS := ezp_msg_buffer_test ezp_byte_buffer_test
ALL_RUNS := $(ALL_TESTS:%=run-%)

all: $(ALL_TESTS)

run-all: $(ALL_RUNS)

run-%: %
	./build/$</$<

ezp_msg_buffer_test:
	TEST_NAME=$@ MSG_TABLE=foobarpingpong.h $(MAKE) -e -f $(TEMPLATE)


ezp_byte_buffer_test:
	TEST_NAME=$@ MSG_TABLE=foobarpingpong.h $(MAKE) -e -f $(TEMPLATE)



clean:
	rm -r ./build/