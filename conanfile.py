from conan import ConanFile

class DependenciesRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "PkgConfigDeps", "CMakeToolchain"

    def requirements(self):
        self.requires("freetype/2.13.2")
        self.requires("libjpeg/9e")
        self.requires("tinyxml2/10.0.0")
        self.requires("libpng/1.6.42")
        self.requires("zlib/1.2.13")
        self.requires("libtiff/4.4.0")
        self.requires("libxml2/2.10.3")
        self.requires("libiconv/1.17")
        self.requires("hunspell/1.7.2")
        if self.settings.os == "Windows":
            self.requires("fontconfig/2.15.0")
            self.requires("gettext/1.0")
            self.requires("libgettext/0.26")
            self.tool_requires("winflexbison/2.5.25")

    def configure(self):
        self.options["freetype"].shared = False
        self.options["libjpeg"].shared = False
        self.options["libpng"].shared = False
        self.options["zlib"].shared = False
        self.options["libtiff"].shared = False
        self.options["libxml2"].shared = False
        self.options["libiconv"].shared = False
        self.options["fontconfig"].shared = False
        self.options["tinyxml2"].shared = False
        self.options["hunspell"].shared = True
        self.options["gettext"].shared = False
        self.options["libgettext"].shared = False

    def imports(self):
        self.copy("*.dll", "bin", "bin")
        self.copy("*.dylib", "lib", "lib")
