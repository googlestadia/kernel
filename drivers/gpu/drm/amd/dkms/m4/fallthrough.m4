dnl #
dnl # test if toolchain supports fallthrough
dnl #
AC_DEFUN([AC_AMDGPU_FALLTHROUGH], [
	AC_KERNEL_DO_BACKGROUND([
		AC_KERNEL_TRY_COMPILE([
		], [
			switch (0) {
				default:
					fallthrough;
				case 0:
					break;
			}

			return 0;
		], [
			AC_DEFINE(HAVE_FALLTHROUGH_SUPPORT, 1,
				[fallthrough is supported])
		])
	])
])
