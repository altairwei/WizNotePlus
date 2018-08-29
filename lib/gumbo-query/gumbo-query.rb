class GumboQuery < Formula
  homepage "https://github.com/Falven/gumbo-query"
  url "https://github.com/Falven/gumbo-query.git", :using => :git
  sha256 "e772da6f700b29367d4ab7a7d462d89c0a6fff0e238c725eda8d9d7c44025839"

  depends_on "cmake" => :build

  def install
    system "cd build"
    system "cmake .."
    system "make"
    system "make install -DCMAKE_INSTALL_PREFIX:/usr/local/Cellar/gumbo-query/"
  end

  def test
    system "make test"
  end
end
