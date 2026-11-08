/* Local route: the same RXBIN owner process runs inside a dedicated Worker. */
export class LocalRoute {
  constructor(workerUrl = new URL('./local-worker.mjs', import.meta.url)) {
    this.workerUrl = workerUrl;
    this.worker = null;
  }
  async start(onFrame) {
    this.onFrame = onFrame;
    this.worker = new Worker(this.workerUrl, {type: 'module'});
    const session = `local-${crypto.randomUUID()}`;
    await new Promise((resolve, reject) => {
      let ready = false;
      this.worker.onmessage = ({data}) => {
        if (data.type === 'frame') {
          try { onFrame(data.frame); if (!ready && data.frame.type === 'view') { ready = true; resolve(); } }
          catch (error) { reject(error); this.stop(); }
        } else if (data.type === 'failure') {
          if (ready) onFrame({version: 1, type: 'refusal', session, revision: 0,
            result: {code: 'host_error', operation: 'browser', message: data.message}});
          else reject(new Error(data.message));
          this.stop();
        }
      };
      this.worker.onerror = event => {
        if (ready) onFrame({version: 1, type: 'refusal', session, revision: 0,
          result: {code: 'host_error', operation: 'browser', message: event.message}});
        else reject(new Error(event.message));
        this.stop();
      };
      this.worker.postMessage({type: 'start', session});
    });
  }
  async send(input) {
    if (!this.worker) throw new Error('Local UI Worker stopped');
    this.worker.postMessage({type: 'input', input});
  }
  stop() { this.worker?.terminate(); this.worker = null; }
  async restart(onFrame = this.onFrame) { this.stop(); return this.start(onFrame); }
}
