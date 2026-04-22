import { QueueReader } from './queue_reader';
import { QueueWriter } from './queue_writer';

/**
 * Interface for both the read and write side of a Queue.
 * @param T - The data to store in the Queue.
 */
export interface AbstractQueue<T> extends QueueReader<T>, QueueWriter<T> {}
