
intps.dll: dlldata.obj int_p.obj int_i.obj
	link /dll /out:intps.dll /def:intps.def /entry:DllMain dlldata.obj int_p.obj int_i.obj \
		kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib \

.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0400 /DREGISTER_PROXY_DLL \
		$<

clean:
	@del intps.dll
	@del intps.lib
	@del intps.exp
	@del dlldata.obj
	@del int_p.obj
	@del int_i.obj
