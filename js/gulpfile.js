var gulp = require('gulp')

gulp.task('default', function () {
  gulp.src(['bower.json'])
    .pipe(gulp.dest('chuck-package'))
  gulp.src(['lib/chuck.js*'])
      .pipe(gulp.dest('chuck-package/lib'))
})