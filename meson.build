project(
	'dbs26', 'c',
	version : '0.0.1',
	meson_version : '>=1.1.0',
	default_options : [
		'buildtype=release',
		'b_lto=true',
		'b_ndebug=true',
		'debug=false',
		'm_arch=native',
		'm_tune=native',
		'optimization=3',
		'strip=true',
		'stupid=false',
		'suffix=false',
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
	m_arch = ''
	m_tune = ''
elif host_sys != 'darwin'
	m_arch = get_option('m_arch')
	if m_arch != ''
		add_project_arguments(
			'-march=' + m_arch,
			language: 'c')
	endif

	m_tune = get_option('m_tune')
	if m_tune != ''
		add_project_arguments(
			'-mtune=' + m_tune,
			language: 'c')
	endif
else
	m_arch = ''
	m_tune = ''
endif

if host_cc.get_id() == 'gcc'
	if get_option('warning_level') == 'everything'
		add_project_arguments(
			'-Wno-c++-compat',
			language: 'c')
	endif
endif

if get_option('stupid')
	suffix_1 = '_' + host_cc.get_id() + '-' + host_cc.version() + '_-O' + get_option('optimization')
	suffix_2 = (m_arch == '' ? '' : '_-march=') + m_arch + (m_tune == '' ? '' : '_-mtune=') + m_tune
	exe_name = 'dbs26' + suffix_1 + suffix_2
elif get_option('suffix')
	exe_name = 'dbs26_' + host_cc.get_id() + '-' + host_cc.version().split('.')[0]
else
	exe_name = 'dbs26'
endif

if meson.is_cross_build() and build_sys == 'darwin' and host_sys == 'darwin'
	host_exe = executable(
		exe_name + '-' + host_cpu,
		'src/args.c',
		'src/dbs26.c',
		install : false,
		native : false)

	build_exe = executable(
		exe_name + '-' + build_cpu,
		'src/args.c',
		'src/dbs26.c',
		install : false,
		native : true)

	universal_exe = custom_target(
		exe_name + '-universal',
		output : exe_name + '-universal',
		input : [ host_exe, build_exe ],
		command : [
			meson.get_external_property('lipo', 'llvm-lipo'),
			'-create', '-output', '@OUTPUT@', '@INPUT@'
		],
		install : false)

	custom_target(
		exe_name,
		output : exe_name,
		input : [ universal_exe, host_exe, build_exe ],
		command : [
			'bash', '-c', 'install --strip-program="$1" -sT "$3" "$2"; rm -f "$3" "$4" "$5"',
			'--', meson.get_external_property('strip', 'llvm-strip'), '@OUTPUT@', '@INPUT@'
		],
		install : true,
		install_dir : get_option('bindir'),
		build_by_default : true)
else
	executable(
		exe_name,
		'src/args.c',
		'src/dbs26.c',
		install : true)
endif

summary({
	'executable': exe_name,
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
