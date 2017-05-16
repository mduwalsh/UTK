package org.myorg;

import java.io.*;
import java.util.*;

import org.apache.hadoop.fs.Path;
import org.apache.hadoop.filecache.DistributedCache;
import org.apache.hadoop.conf.*;
import org.apache.hadoop.io.*;
import org.apache.hadoop.mapred.*;
import org.apache.hadoop.util.*;

public class LineIndexer extends Configured implements Tool {

   public static class Map extends MapReduceBase implements Mapper<LongWritable, Text, Text, Text> {

     private Set<String> patternsToSkip = new HashSet<String>();
     private Text word = new Text();
     private String inputFile;

     public void configure(JobConf job) {
       //inputFile = job.get("map.input.file");               // old api method to get input file

       if (job.getBoolean("lineindexer.skip.patterns", false)) {
         Path[] patternsFiles = new Path[0];
         try {
           patternsFiles = DistributedCache.getLocalCacheFiles(job);
         } catch (IOException ioe) {
           System.err.println("Caught exception while getting cached files: " + StringUtils.stringifyException(ioe));
         }
         for (Path patternsFile : patternsFiles) {
           parseSkipFile(patternsFile);
         }
       }
     }

     private void parseSkipFile(Path patternsFile) {   // parses stopwords file and adds the words to skip list
       try {
         BufferedReader fis = new BufferedReader(new FileReader(patternsFile.toString()));
         String pattern = null;
         while ((pattern = fis.readLine()) != null) {
           patternsToSkip.add(pattern.trim().toLowerCase());
         }
       } catch (IOException ioe) {
         System.err.println("Caught exception while parsing the cached file '" + patternsFile + "' : " + StringUtils.stringifyException(ioe));
       }
     }

     public void map(LongWritable key, Text value, OutputCollector<Text, Text> output, Reporter reporter) throws IOException {
		 int lineno = -1, wordno = -1;
       StringBuilder str = new StringBuilder();
       String line = value.toString().toLowerCase();              // convert all text value to lower string
       FileSplit fsplit = (FileSplit) reporter.getInputSplit();   // get file split from reporter object
       inputFile = fsplit.getPath().getName();                    // get file name from file split object

      // for (String pattern : patternsToSkip) {                    // skip words in stop words list
        // line = line.replaceAll(pattern, "");
       //}

       StringTokenizer tokenizer = new StringTokenizer(line, " \t\n\r\f,.:;?![]'");
       if(tokenizer.hasMoreTokens()){  // read first token
         lineno = Integer.parseInt(tokenizer.nextToken().toString());  // parse first token as line no.
       }
       wordno = 0;
	   String keyString = new String();
       while (tokenizer.hasMoreTokens()) {  // read other tokens as words
		 wordno++;                  // increase the word offset
                 keyString = tokenizer.nextToken().trim();
		 if(keyString.isEmpty() || patternsToSkip.contains(keyString)) continue;   // skip the word if it is blank or is in skip list
		 str.setLength(0);
         str.append(inputFile); str.append("@"); str.append(lineno); str.append("@"); str.append(wordno);  // create a format of 'filename@lineno@wordno'
         word.set(keyString);
         output.collect(word, new Text(str.toString()));
       }

       /*if ((++numRecords % 100) == 0) {
         reporter.setStatus("Finished processing " + numRecords + " records " + "from the input file: " + inputFile);
       }*/
     }
   }

   public static class Reduce extends MapReduceBase implements Reducer<Text, Text, Text, Text> {
     public void reduce(Text key, Iterator<Text> values, OutputCollector<Text, Text> output, Reporter reporter) throws IOException {
       StringBuilder str = new StringBuilder();
       int index;
	   index = 0;
       while (values.hasNext()) {   // create comma delimited values like 'value1,value2,value3'
         //str.setLength(0);
		 if(index > 0){  // if it is first value, then skip appending ','
           str.append(",");
         }         
         str.append(values.next().toString());
		 index++;
       }
       output.collect(key, new Text(str.toString()));
     }
   }

   public int run(String[] args) throws Exception {
     JobConf conf = new JobConf(getConf(), LineIndexer.class);
     conf.setJobName("lineindexer");

     conf.setOutputKeyClass(Text.class);
     conf.setOutputValueClass(Text.class);

     conf.setMapperClass(Map.class);
     conf.setCombinerClass(Reduce.class);
     conf.setReducerClass(Reduce.class);

     conf.setInputFormat(TextInputFormat.class);
     conf.setOutputFormat(TextOutputFormat.class);

     List<String> other_args = new ArrayList<String>();
     for (int i=0; i < args.length; ++i) {
       if ("-skip".equals(args[i])) {
         DistributedCache.addCacheFile(new Path(args[++i]).toUri(), conf);
         conf.setBoolean("lineindexer.skip.patterns", true);
       } else {
         other_args.add(args[i]);
       }
     }

     FileInputFormat.setInputPaths(conf, new Path(other_args.get(0)));
     FileOutputFormat.setOutputPath(conf, new Path(other_args.get(1)));

     JobClient.runJob(conf);
     return 0;
   }

   public static void main(String[] args) throws Exception {
     int res = ToolRunner.run(new Configuration(), new LineIndexer(), args);
     System.exit(res);
   }
}
