module JubaRuby
  class MulticalssResult
    class Score
      attr_accessor :score, :label
      def initialize(label, score)
        @label = label
        @score = score
      end
    end

    attr_reader :prediction, :results

    def initialize(results)
      @results = results
      if results.empty?
        @prediction = nil
      else
        @prediction = results.max{ |x, y| x.score <=> y.score }
      end
    end
  end

end # module JubaRuby
