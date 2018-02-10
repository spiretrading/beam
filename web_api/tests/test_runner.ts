import * as Alsatian from "alsatian";

const testSet = Alsatian.TestSet.create();
testSet.addTestsFromFiles("./library/beam/tests/*.js");
testSet.addTestsFromFiles("./library/beam/tests/definitions/*.js");
const testRunner = new Alsatian.TestRunner();
testRunner.outputStream.pipe(process.stdout);
testRunner.run(testSet);
