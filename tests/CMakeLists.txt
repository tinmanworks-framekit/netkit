add_executable(netkit_contracts_test
    test_transport_contracts.cpp
)

target_link_libraries(netkit_contracts_test PRIVATE NetKit::Transport)
add_test(NAME netkit_contracts_test COMMAND netkit_contracts_test)
