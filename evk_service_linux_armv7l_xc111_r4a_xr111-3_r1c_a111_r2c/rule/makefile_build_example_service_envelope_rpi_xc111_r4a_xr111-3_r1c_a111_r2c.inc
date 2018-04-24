BUILD_ALL += out/example_service_envelope_rpi_xc111_r4a_xr111-3_r1c_a111_r2c

out/example_service_envelope_rpi_xc111_r4a_xr111-3_r1c_a111_r2c : \
					out/example_service_envelope.o \
					libacconeer.a \
					libacconeer_a111_r2c.a \
					libacc_local_server.a \
					libacc_message_driver_dummy.a \
					out/libcustomer.a \
					libacc_envelope.a \
					libacc_power_bins.a \
					libacc_range.a \
					libacc_service.a \
					out/acc_board_rpi_xc111_r4a_xr111-3_r1c.o
	@echo "    Linking $(notdir $@)"
	@mkdir -p out
	@$(LINK.o) -Wl,--start-group $^ -Wl,--end-group $(LOADLIBES) $(LDLIBS) -o $@
