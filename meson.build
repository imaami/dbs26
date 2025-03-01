project('dbs26', 'c',
  version : '0.0.1',
  default_options : ['warning_level=3'])

executable('dbs26',
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
}, section: 'Directories')

hcc = meson.get_compiler('c', native: false)
hsys = host_machine.cpu() + '-' + host_machine.system()
hcom = hcc.get_id() + ' ' + hcc.version() + ' / ' + hcc.get_linker_id()

summary({
  'platform': hsys,
  'compiler': hcom
}, section: 'Host')

if meson.is_cross_build()
  bcc = meson.get_compiler('c', native: true)
  bsys = build_machine.cpu() + '-' + build_machine.system()
  bcom = bcc.get_id() + ' ' + bcc.version() + ' / ' + bcc.get_linker_id()
  summary({
    'platform': bsys,
    'compiler': bcom
  }, section: 'Build host')
endif

#hstd = hcc.first_supported_argument(
#  '-std=gnu23', '-std=c23', '-std=gnu2x', '-std=c2x',
#  '-std=gnu18', '-std=c18', '-std=gnu17', '-std=c17',
#  '-std=gnu11', '-std=c11', '-std=gnu1x', '-std=c1x')
#bstd = bcc.first_supported_argument(
#  '-std=gnu23', '-std=c23', '-std=gnu2x', '-std=c2x',
#  '-std=gnu18', '-std=c18', '-std=gnu17', '-std=c17',
#  '-std=gnu11', '-std=c11', '-std=gnu1x', '-std=c1x')
