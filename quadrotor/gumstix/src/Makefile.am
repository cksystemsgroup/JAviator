
#include base project directory to get the shared headers
AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_builddir) $(CONTROLLER_CFLAGS) \
              -I$(top_srcdir)/.. -I$(top_builddir)/..

noinst_LIBRARIES = libcontroller-platform.a
bin_PROGRAMS = control
control_LDADD = libcontroller-platform.a
control_LDFLAGS = -lrt -lm -lpthread
control_SOURCES = \
    average_filter.c \
    communication.c \
    controller.c \
    control_loop.c \
    ctrl_params.c \
    inertial_data.c \
    inertial_port.c \
    javiator_port.c \
    kalman_filter.c \
    low_pass_filter.c \
    main.c \
    median_filter.c \
    pidd_controller.c \
    rotation_matrix.c \
    serial_channel.c \
    socket_channel.c \
    spi_channel.c \
    spi_dev_channel.c \
    terminal_port.c \
    ubisense_port.c \
    udp_socket_channel.c \
    us_timer.c

libcontroller_platform_a_SOURCES =

objectlist:
	for obj in `echo $(libcontroller_a_OBJECTS)`; do echo $(BASEDIR)/$(subdir)/$$obj >> $(OBJECTLIST); done
	for obj in `echo $(libcontroller_platform_a_OBJECTS)`; do echo $(BASEDIR)/$(subdir)/$$obj >> $(OBJECTLIST); done

