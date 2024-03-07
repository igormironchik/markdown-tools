from conan import ConanFile

class DependenciesRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps"

    def requirements(self):
        self.requires("freetype/2.13.2")
        self.requires("libjpeg/9e")
        self.requires("libpng/1.6.42")
        self.requires("zlib/1.2.13")
        self.requires("libtiff/4.4.0")
        self.requires("libxml2/2.10.3")
        self.requires("libiconv/1.17")
        if self.settings.os == "Windows":
            self.requires("fontconfig/2.15.0")

    def configure(self):
        self.options["freetype"].shared = False
        self.options["libjpeg"].shared = False
        self.options["libpng"].shared = False
        self.options["zlib"].shared = False
        self.options["libtiff"].shared = False
        self.options["libxml2"].shared = False
        self.options["libiconv"].shared = False
        self.options["fontconfig"].shared = False

    def imports(self):
        self.copy("*.dll", "bin", "bin")
        self.copy("*.dylib", "lib", "lib")
