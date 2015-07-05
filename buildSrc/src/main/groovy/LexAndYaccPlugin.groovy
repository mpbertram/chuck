import org.gradle.api.*
import org.gradle.api.file.*
import org.gradle.api.tasks.*

class LexCompiler extends DefaultTask {
  @InputFiles FileCollection lexFiles
  @OutputDirectory File sourceDir
  @OutputDirectory File headerDir

  @TaskAction
  void processLexFiles() {
    lexFiles.each { file ->
      project.exec {
        commandLine = ['flex', '-o', new File(sourceDir, file.name - '.lex' + '.yy.c').absolutePath,
          file.absolutePath]
      }
    }
  }
}

class YaccCompiler extends DefaultTask {
  @InputFiles FileCollection yaccFiles
  @OutputDirectory File sourceDir
  @OutputDirectory File headerDir

  @TaskAction
  void processYaccFiles() {
    yaccFiles.each { file ->
      project.exec {
        commandLine = ['bison', '-dv', '-o', new File(sourceDir, file.name - '.y' + '.tab.c'),
          file.absolutePath]
      }
    }
  }
}

class LexAndYaccPlugin implements Plugin<Project> {
  void apply(Project project) {
    project.task('yacc', type: YaccCompiler) {
      sourceDir = project.file("${project.buildDir}/src/generated/yacc")
      headerDir = project.file("${project.buildDir}/src/generated/yacc")
      yaccFiles = project.files('src/chuck.y')
    }

    project.task('lex', type: LexCompiler) {
      sourceDir = project.file("${project.buildDir}/src/generated/lex")
      headerDir = project.file("${project.buildDir}/src/generated/lex")
      lexFiles = project.files('src/chuck.lex')
    }
  }
}
