
AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_builddir) $(CONTROLLER_CFLAGS)

noinst_LIBRARIES = libcontroller-platform.a
bin_PROGRAMS = control
control_LDADD = libcontroller-platform.a
control_LDFLAGS = -lrt -lm
control_SOURCES = \
    butter_filter.c \
    channel.c \
    communication.c \
    controller.c \
    control_loop.c \
    inertial_port.c \
    javiator_port.c \
    kalman_filter.c \
    main.c \
    navigation.c \
    param.c \
    pid_controller.c \
    pidd_controller.c \
    pwm_signals.c \
    sensor_data.c \
    serial_channel.c \
    socket_channel.c \
    spi_channel.c \
    terminal_port.c \
    trim.c \
    utimer.c

libcontroller_platform_a_SOURCES =

objectlist:
	for obj in `echo $(libcontroller_a_OBJECTS)`; do echo $(BASEDIR)/$(subdir)/$$obj >> $(OBJECTLIST); done
	for obj in `echo $(libcontroller_platform_a_OBJECTS)`; do echo $(BASEDIR)/$(subdir)/$$obj >> $(OBJECTLIST); done

