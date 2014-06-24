#0.2.4

The following changes:

1. fwdpp/IO.hpp and fwdpp/IO.tcc were updated to include the ability to read/write binary-format data to/from gzip files.  The gzip compression uses [zlib](http://zlib.net).  Note that the combination of POSIX file locking + gzip output requires more work than "plain" binary output + file locking.  Please see the new examples.  Differences in how gztell vs. ftell mean that the index files generated are different.  See my [tutorial](https://github.com/molpopgen/BigDataFormats) on on "big data" file formats for more detail.
2.  examples/pfix.cc was added.  This estimates the fixation probability of a mutation subject to genic selection in a constant-sized population of N diploids.
3.  examples/diploid_gzbinaryIO.cc and examples/diploid_gzbinaryIO_ind.cc were added.  They illustrate the new functions mentioned in point 1.
4. The folder "test" was added.  This contains several Rmd ([R Markdown](http://rmarkdown.rstudio.com/)) documents that you can process either  in [R studio](http://www.rstudio.com/) or plain old [R](http://www.r-project.org).  In either case, you'll need [knitr](http://cran.r-project.org/web/packages/knitr/index.html) installed.