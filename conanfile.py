from conan import ConanFile

class DependenciesRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "PkgConfigDeps"

    def requirements(self):
        self.requires("zlib/1.3.1")
        self.requires("hunspell/1.7.2")
        self.requires("tinyxml2/10.0.0")
        if self.settings.os == "Windows":
            self.requires("fontconfig/2.15.0")

    def configure(self):
        self.options["zlib"].shared = False
        self.options["hunspell"].shared = True
        self.options["tinyxml2"].shared = False
        self.options["fontconfig"].shared = False

    def imports(self):
        self.copy("*.dll", "bin", "bin")
        self.copy("*.dylib", "lib", "lib")
