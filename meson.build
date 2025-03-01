project(
	'dbs26', 'c',
	version : '0.0.1',
	default_options : ['warning_level=3'],
	meson_version : '>=1.1.0')

env = environment()
host_cc = meson.get_compiler('c', native: false)
build_cc = meson.get_compiler('c', native: true)
host_cpu = host_machine.cpu_family()
build_cpu = build_machine.cpu_family()
host_sys = host_machine.system()
build_sys = build_machine.system()

if host_sys == 'windows' and host_cc.get_id() == 'msvc'
	msvc_path = get_option('msvc_path')
	if msvc_path != ''
		if host_cpu == 'x86_64'
			msvc_bin = msvc_path + '/bin/x64'
		elif host_cpu == 'aarch64'
			msvc_bin = msvc_path + '/bin/arm64'
		else
			msvc_bin = msvc_path + '/bin/' + host_cpu
		endif
		env.prepend('PATH', msvc_bin)
	endif

	add_project_arguments(
		'/utf-8', '/nologo',
		'/TC', '/std:clatest',
		'/experimental:c11atomics',
		'/Oi', '/GL', '/GF', '/Zo-',
		language: 'c', native: false)
	add_project_link_arguments(
		'/LTCG',
		language: 'c', native: false)
endif

executable(
	'dbs26',
	'src/args.c',
	'src/dbs26.c',
	install : true)

summary({
	'prefix': get_option('prefix'),
	'bindir': get_option('bindir'),
	'includedir': get_option('includedir'),
	'libdir': get_option('libdir'),
	'mandir': get_option('mandir'),
	'sysconfdir': get_option('sysconfdir'),
	'msvc_path': msvc_path,
}, section: 'Directories')


summary({
	'platform': host_cpu + '-' + host_sys,
	'compiler': host_cc.get_id() + ' ' + host_cc.version() + ' / ' + host_cc.get_linker_id()
}, section: meson.is_cross_build() ? 'Host machine' : 'Machine')

if meson.is_cross_build()
	summary({
		'platform': build_cpu + '-' + build_sys,
		'compiler': build_cc.get_id() + ' ' + build_cc.version() + ' / ' + build_cc.get_linker_id()
	}, section: 'Build machine')
endif
