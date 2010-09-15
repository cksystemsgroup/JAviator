
#include base project directory to get the shared headers
AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_builddir) $(CONTROLLER_CFLAGS) \
              -I$(top_srcdir)/.. -I$(top_builddir)/..

noinst_LIBRARIES = libcontroller-platform.a
bin_PROGRAMS = control
control_LDADD = libcontroller-platform.a
control_LDFLAGS = -lrt -lm -lpthread
control_SOURCES = \
    attitude_ekf.c \
    average_filter.c \
    communication.c \
    controller.c \
    control_loop.c \
    fir_lp_filter.c \
    iir_lp_filter.c \
    javiator_port.c \
    main.c \
    median_filter.c \
    outlier_filter.c \
    pidd_controller.c \
    position_ekf.c \
    serial_channel.c \
    tcp_socket_channel.c \
    terminal_port.c \
    transformation.c \
    ubisense_port.c \
    udp_socket_channel.c \
    us_timer.c

libcontroller_platform_a_SOURCES =

objectlist:
	for obj in `echo $(libcontroller_a_OBJECTS)`; do echo $(BASEDIR)/$(subdir)/$$obj >> $(OBJECTLIST); done
	for obj in `echo $(libcontroller_platform_a_OBJECTS)`; do echo $(BASEDIR)/$(subdir)/$$obj >> $(OBJECTLIST); done

