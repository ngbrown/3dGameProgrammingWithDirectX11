# Learning DirectX 11 with Windows 8.1 SDK 8.1

My journey through the book, [Introduction to 3D Game Programming with DirectX 11 by Frank D. Luna](http://www.d3dcoder.net/d3d11.htm).

The book implements the examples against the older [June 2010 DirectX SDK](https://www.microsoft.com/download/details.aspx?id=6812).  Instead, I am implementing against the [Windows 8.1 SDK](http://go.microsoft.com/fwlink/p/?linkid=253588) installed as part of [Visual Studio 2013](https://www.visualstudio.com/).

* DXERR.LIB is not longer part of the Windows SDK, so a streamlined version of the library [from Chuck Walbourn (MSFT)](http://blogs.msdn.com/b/chuckw/archive/2012/04/24/where-s-dxerr-lib.aspx) is included.
* Include `D3DX11INLINE` and `ShaderHelper` from [Alex Jadczak](https://alexjadczak.wordpress.com/2014/05/18/updating-directx-11-to-windows-8-0-sdk/).
