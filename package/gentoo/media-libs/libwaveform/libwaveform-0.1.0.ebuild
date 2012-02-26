# Copyright 1999-2006 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

RESTRICT="nomirror"
IUSE="lv2 no_help"
DESCRIPTION="Libwaveform provides efficient display of audio waveforms for Gtk applications."
HOMEPAGE="http://ayyi.org/"
SRC_URI="http://ayyi.org/files/${P}.tar.gz"
RESTRICT="nomirror"

LICENSE="GPL-3"
KEYWORDS="x86"
SLOT="0"

DEPEND=">=media-libs/libsndfile-1.0.10
	>=x11-libs/gtk+-2.6
	>=x11-libs/gtkglext-1.0"

#src_compile() {
#	local myconf
#
#	econf \
#		${myconf} \
#		|| die
#	emake || die
#}

src_install() {
	make DESTDIR=${D} install || die
	dodoc NEWS ChangeLog
}

