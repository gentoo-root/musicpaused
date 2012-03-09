# media-sound/musicpaused/musicpaused-9999.ebuild
EAPI=3

inherit git-2

DESCRIPTION="Pause music when headphones are unplugged"
HOMEPAGE="https://github.com/gentoo-root/musicpaused"
EGIT_REPO_URI="git://github.com/gentoo-root/musicpaused.git"

LICENSE="GPL-3"
SLOT="0"
KEYWORDS="~amd64 ~x86"
IUSE=""

RDEPEND="sys-apps/dbus
	media-sound/jacklistener"
DEPEND="${RDEPEND}"

src_install() {
	emake DESTDIR="${D}" install || die
	dodoc AUTHORS NEWS README* ChangeLog
}
