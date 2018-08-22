project('afsk', 'c',
	version: 'dev',
	default_options : [ 'warning_level=3',
						'c_std=c11',
                      	'buildtype=release'])

cc = meson.get_compiler('c')

m_dep 		= cc.find_library('m', required : true)
sndfile_dep = dependency('sndfile', required: true)
#alsa 	 = dependency('alsa')

sources = ['modem.c', 'crc.c', 'hdlc.c', 'ringbuffer.c', 'main.c']

executable('afsk',
	sources,
	dependencies: [sndfile_dep, m_dep]
)
