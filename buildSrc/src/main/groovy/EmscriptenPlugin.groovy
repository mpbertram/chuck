import org.gradle.api.*
import org.gradle.api.plugins.*
import org.gradle.api.file.*
import org.gradle.api.tasks.*
import org.gradle.api.tasks.incremental.IncrementalTaskInputs

class EmscriptenBaseTask extends DefaultTask {
  def emccPath = new File(System.getenv("EMSCRIPTEN"), 'emcc').absolutePath
}

class EmscriptenCompiler extends EmscriptenBaseTask {
  @InputFiles FileCollection emscriptenFiles
  @OutputDirectory File outputDirectory

  def replaceExtension(fileName) {
    return fileName.replaceFirst(/\.(c|cpp)$/, '.o')
  }

  @TaskAction
  void processEmscriptenFiles(IncrementalTaskInputs inputs) {
    inputs.outOfDate { change ->
      project.exec {
        // TODO: Make -g configurable, don't hardcode YACC header dir
        commandLine = [emccPath, '-Isrc', '-Isrc/lo', '-Ibuild/src/generated/yacc', '-g', '-o',
          new File(outputDirectory, replaceExtension(change.file.name)).absolutePath,
          change.file.absolutePath]
      }
    }

    inputs.removed { change ->
      new File(outputDirectory, replaceExtension(change.file.name)).delete()
    }
  }
}

class EmscriptenLinker extends EmscriptenBaseTask {
  @InputFiles FileCollection emscriptenFiles
  @OutputDirectory File outputDirectory

  @TaskAction
  void processEmscriptenFiles() {
    project.exec {
      // TODO: Make -g and SAFE_HEAP configurable
      commandLine = [emccPath, '-g', '-s', 'EXPORTED_FUNCTIONS=["_executeCode"]', '--js-library',
        'src/emscripten/libraries/webaudio.js', '-s', 'SAFE_HEAP=1', '-s', 'DEMANGLE_SUPPORT=1', '-o',
        new File(outputDirectory, 'chuck.js').absolutePath] + emscriptenFiles.collect { it.absolutePath }
    }
  }
}

class EmscriptenPlugin implements Plugin<Project> {
  void apply(Project project) {
    project.task('emscriptenYacc', type: EmscriptenCompiler) {
      outputDirectory = project.file("${project.buildDir}/emscripten")
      emscriptenFiles = project.fileTree(project.tasks.yacc.sourceDir)
        .include('chuck.tab.c')
      dependsOn(project.tasks.yacc)
    }

    project.task('emscriptenLex', type: EmscriptenCompiler) {
      outputDirectory = project.file("${project.buildDir}/emscripten")
      emscriptenFiles = project.fileTree(project.tasks.lex.sourceDir)
        .include('chuck.yy.c')
      dependsOn(project.tasks.lex)
      dependsOn(project.tasks.yacc)
    }

    project.task('emscriptenCompile', type: EmscriptenCompiler) {
      outputDirectory = project.file("${project.buildDir}/emscripten")
      emscriptenFiles = project.fileTree(dir: 'src')
        .include('*.c')
        .include('*.cpp')
        .exclude('rtaudio_c.cpp')
        .exclude('digiio_rtaudio.cpp')
        .exclude('util_sndfile.c')
        .exclude('chuck_win32.c')

      dependsOn(project.tasks.yacc)
    }

    project.task('emscripten', type: EmscriptenLinker) {
      outputDirectory = project.file("${project.buildDir}/js")
      emscriptenFiles = project.tasks.emscriptenYacc.outputs.files.asFileTree + \
        project.tasks.emscriptenLex.outputs.files.asFileTree + \
        project.tasks.emscriptenCompile.outputs.files.asFileTree
    }
  }
}
