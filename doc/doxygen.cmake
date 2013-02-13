# doxygen helper

set(ENV{CURSORD_ROOT} ${PDIR})
message(${PDIR})
execute_process(
	COMMAND doxygen "${PDIR}/doc/doxygen")
