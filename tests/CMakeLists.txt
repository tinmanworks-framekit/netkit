add_executable(netkit_contracts_test
    test_transport_contracts.cpp
)

target_link_libraries(netkit_contracts_test PRIVATE NetKit::Transport)
add_test(NAME netkit_contracts_test COMMAND netkit_contracts_test)

add_executable(netkit_control_channel_test
    test_control_channel.cpp
)

target_link_libraries(netkit_control_channel_test PRIVATE NetKit::Transport)
add_test(NAME netkit_control_channel_test COMMAND netkit_control_channel_test)
