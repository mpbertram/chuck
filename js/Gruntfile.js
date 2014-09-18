module.exports = function (grunt) {
  grunt.loadNpmTasks('grunt-contrib-coffee');
  grunt.loadNpmTasks('grunt-shell');
  grunt.loadNpmTasks('grunt-contrib-copy');
  grunt.loadNpmTasks('grunt-zetzer')
  grunt.loadNpmTasks('grunt-contrib-watch')

  grunt.initConfig({
    coffee: {
      compileExamples: {
        options: { bare: true },
        expand: true,
        flatten: false,
        cwd: 'pages/',
        src: ['**/*.coffee'],
        dest: 'examples/',
        ext: '.js'
      }
    },
    copy: {
      zetzer: {
        files: [
          {
            expand: true, cwd: 'pages', dest: 'examples/', filter: 'isFile',
            src: [
              '**/*.js', '**/*.css'
            ]
          },
          {
            expand: true, cwd: '../src', dest: 'examples/js',
            src: ['chuck.js']
          },
        ]
      }
    },
    zetzer: {
      options: {
        templates: 'pages/templates'
      },
      main: {
        options: {
          env: {
            root: ''
          }
        },
        files: {
          'examples/example1.html': ['pages/example1.dot.html'],
          'examples/example2.html': ['pages/example2.dot.html']
        }
      },
      basic: {
        options: {
          env: {
            root: '../'
          }
        },
        files: [
          {
            expand: true,
            cwd: 'pages',
            src: 'basic/**/*.dot.html',
            dest: 'examples/',
            ext: '.html',
            filter: 'isFile'
          }
        ]
      }
    },
    watch: {
      files: [
        'pages/**/*.dot.html', 'pages/**/*.coffee',
        'Gruntfile.js'
      ],
      tasks: ['default']
    }
  });

  grunt.registerTask('default', ['coffee', 'copy', 'zetzer']);
};
