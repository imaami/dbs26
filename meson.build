project(
	'dbs26', 'c',
	version : '0.0.1',
	meson_version : '>=1.1.0',
	default_options : [
		'm_arch=native',
		'buildtype=release',
		'debug=false',
		'optimization=3',
		'strip=true',
		'm_tune=native',
		'unique=true',
		'warning_level=everything',
	],
)

host_cc = meson.get_compiler('c', native: false)
build_cc = meson.get_compiler('c', native: true)
host_cpu = host_machine.cpu_family()
build_cpu = build_machine.cpu_family()
host_sys = host_machine.system()
build_sys = build_machine.system()

if host_cc.get_id() == 'msvc'
	add_project_arguments(
		'/utf-8', '/nologo',
		'/TC', '/std:clatest',
		'/experimental:c11atomics',
		'/Oi', '/GL', '/GF', '/Zo-',
		language: 'c',
		native: build_cc.get_id() == 'msvc')
	add_project_link_arguments(
		'/LTCG',
		language: 'c',
		native: build_cc.get_id() == 'msvc')
endif

if get_option('unique')
	exe_name = 'dbs26_' + host_cc.get_id() + '-' + host_cc.version() + '_O' + get_option('optimization')
else
	exe_name = 'dbs26'
endif

executable(
	exe_name,
	'src/args.c',
	'src/dbs26.c',
	install : true)

summary({
	'build output': exe_name,
	'prefix': get_option('prefix'),
	'bindir': get_option('bindir'),
	'includedir': get_option('includedir'),
	'libdir': get_option('libdir'),
	'mandir': get_option('mandir'),
	'sysconfdir': get_option('sysconfdir'),
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
